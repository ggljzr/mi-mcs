# MI-MCS

Paralelní počítání prvočísel pomocí [Cilk Plus](https://www.cilkplus.org).

## Kompilace

Pokud defaultní G++ (`which g++`) nepodporuje rozšíření Cilk, je nutné v Makefile specifikovat cestu k Cilk kompilátoru.

## Použití

Výpočet prvočísel do zadané hranice (s maximálním počtem vláken):

```
# vypocet prvocisel do 10^9
$ primes 1000000000
```

Libovolný počet vláken je možné nastavit pomocí přepínače `-t`:

```
# vypocet prvocisel do 100 s jednim vlaknem
$ primes -t 1 100
```

Nalezená prvočísla jsou uložena v souboru `primes.txt`.

## Algoritmus

K výpočtu je použito [Eratosthenovo síto](https://en.wikipedia.org/wiki/Sieve_of_Eratosthenes). Tento algoritmus je založen na odstraňování násobků již nalezených prvočísel.

Jako síto jsem nejprve použil binární pole délky *n*, kde *n* je uživatelem zadaný horní limit hledaných prvočísel. V tomto poli jsem postupně vyškrtal násobky prvočísel. Na konci tedy v poli zůstanou jedničky pouze na prvočíselných indexech.

Dále jsem algoritmus upravil aby pracoval pouze s lichými čísly (tedy zpracovával pouze liché násobky). Díky tomu se zmenšily paměťové nároky a došlo také ke zrychlení programu. Sekvenční tvorba síta tedy vypadá takto:

```cpp
unsigned int sieve_size = (n / 2);
char * sieve = new char[sieve_size];

for(int i = 0; i < sieve_size; i++)
    sieve[i] = 1;

for(unsigned int i = 3; i <= sqrt(n); i += 2)
{
    if(sieve[i / 2] == 1)
    {
        unsigned int inc = 2 * i; //preskakovani sudych nasobku
        for(unsigned int p = i * i; p <= n; p += inc)
        {
            sieve[p / 2] = 0;
        }
    }
}
```

Takovýto kód lze poměrně jednoduše paralelizovat pomocí klíčového slova [cilk_for](https://www.cilkplus.org/tutorial-cilk-plus-keywords#cilk_for).

### Paralelizace

Paralelizaci vytváření síta jsem provedl velmi přímočaře, pouze jsem doplnil `cilk_for` k cyklu, který provádí odstraňování násobků:

```cpp
cilk_for(unsigned int p = i * i; p <= n; p += inc)
{
    sieve[p / 2] = 0;
}
```

Paralelizace vnějšího cyklu se nejevila příliš efektivní, pravděpodobně kvůli relativně malému počtu iterací cyklu a nevyvážené časové náročnosti jednotlivých iterací (iterace pro neprvočíselná *i* hned skončí).

`cilk_for` jsem také použil k počáteční inicializaci síta jedničkami. Zde se tento způsob zdál rychlejší než použití [Cilk Array Notatnion](https://www.cilkplus.org/tutorial-array-notation), tedy `sieve[0:sieve_size] = 1` nebo použití funkce [memset](http://www.cplusplus.com/reference/cstring/memset/).

Po vytvoření síta je potřeba pole projít a nalezená prvočísla zpsat do souboru. Zde jsem opět použil `cilk_for`. Vzhledem k tomu, že zápis prvočísel je citlivý na zpracování více vlákny, použil jsem k jeho realizaci [reducer](https://www.cilkplus.org/tutorial-cilk-plus-reducers), konkrétně `reducer_ostream`:

```cpp
std::ofstream primes_file(PRIMES_FILE_PATH);
cilk::reducer_ostream hyper_out(primes_file);

*hyper_out << 2 << std::endl;
cilk_for(unsigned int i = 3; i <= max_prime; i+= 2)
{
    if(sieve[i / 2] == 1)
    {
    	*hyper_out << i << std::endl;
    }
}

primes_file.close();
```

Reducer zajistí výlučný přístup ke zdrojům pomocí paralelní redukce, a cyklus je tedy možné bez obav paralelizovat pomocí `cilk_for`.

## Segmentovaný algoritmus

Nevýhodou předchozího algoritmu je, že pracuje najednou s velým blokem paměti (pole `sieve`). Ten se zpravidla nejvede do cache procesoru, která je tak využita velmi neefektivně.

Na základě [tohoto článku](http://create.stephan-brumme.com/eratosthenes/) jsem tedy základní algoritmus upravil tak, aby pracoval pouze s vymezenými bloky čísel:

```cpp
    unsigned int sieve_size = (to - from) / 2;
    char * sieve = new char[sieve_size];

    unsigned int sqrtn = (int) sqrt(to);

    for(unsigned int i = 0; i < sieve_size; i++)
        sieve[i] = 1;

    for(unsigned int i = 3; i <= sqrtn; i += 2)
    {
        unsigned int start = MAX(first_odd_multiple(i, from), i * i);

        unsigned int inc = 2 * i;
        for(unsigned int p = start; p <= to; p += inc)
        {
            unsigned int index = (p - from) / 2;
            sieve[index] = 0;
        }
    }
```
Protože už algoritmus nemá k dispozici informaci o dosud nalezených prvočíslech (zná pouze svuj segment), je nutné v daném segmentu postupně projít a vyškrtat všechny liché násobky (algoritmus stále pracuje pouze s lichými čísli) čísel v rozsahu `from` -- `to`. To představuje jediný výraznější rozdíl oproti nesegmentovanému algoritmu.

### Velikost segmentu

Při experimentování s velikostí zpracovávaného segmentu se jako nejvýhodnější jevilo použít segmenty s 65536 čísly. Vzhledem k tomu, že algoritmus pracuje pouze se sudými čísly, je v tomto segmentu alokováno pole (`sieve`) o velikosti 32768 bajtů, což odpovídá L1 cache použitého procesoru (podle `lscpu`).

### Paralelizace segmentovaného algoritmu

Ukázalo se, že paralelizace zpracování jednoho segmentu se příliš nevyplatí, rozhodl jsem se tedy paralelizovat cyklus procházející jednotlivé segmenty:

```cpp
void find_primes_segmented(unsigned int n, unsigned int block_size)
{
    cilk::reducer< cilk::op_max<unsigned int> > max_prime;
    std::ofstream primes_file(PRIMES_FILE_PATH);
    cilk::reducer_ostream hyper_out(primes_file);

    *hyper_out << 2 << std::endl;

    cilk_for(unsigned int i = 2; i <= n; i += block_size)
    {
        unsigned int to = MIN(n, i + block_size);
        max_prime->calc_max(process_segment(i, to, &hyper_out));
    }

    printf("prime: %d\n", max_prime.get_value());
}
```

Opět jsem použil reducer pro zápis nalezených čísel. Ten bylo nutné předat funkci zpracovávající zvolený segment (`process_segment()`).

Tato funkce také vrací nejvyšší prvočíslo ze svého segmentu. Jelikož není zaručeno pořadí zpracovávání jednotlivých iterací, pro nalezení maximálního prvočísla jsem použil reducer s operací maxima. Použití reduceru se zádlo být přibližně stějně rychlé jako uložení maxima pouze z poslední iterace:

```cpp
cilk_for(unsigned int i = 2; i <= n; i += block_size)
{
    unsigned int to = MIN(n, i + block_size);
    unsinged int local = process_segment(i, to, &hyper_out);
    if(to == n)
        max_prime = local;
}
```

Paralelizaci segmentového algoritmu jsem tedy provedl pouze přidáním jednoho `cilk_for` a ošetřením přístupu ke sdíleným zdrojům.

## Závěr

Při paralelizaci algoritmu jsem se snažil využít [serial elision](https://software.intel.com/en-us/blogs/2012/04/07/serial-equivalence-of-cilk-plus-programs) Cilku, tedy implementovat sekvenční algoritmus a vhodným doplněním klíčových slov (`cilk_for`) vytvořit vícevláknovou verzi. 

Tato vlastnost zde není 100% dodržena (například použití reducerů vyžaduje větší zásahy do kódu), nicméně je zachována myšlenka paralelizace sekvenčního algoritmu namísto implementace algoritmu přímo určeného pro paralelní zpracování (kde by například komunikace mezi výpočetními uzly tvořila součást návrhu algoritmu).

## Odkazy

Při tvorbě programu jsem čerpal především z těchto zdrojů:

* [Tutorial](https://www.cilkplus.org/cilk-plus-tutorial) popisující základní vlastnosti Cilku.
* [Článek](http://create.stephan-brumme.com/eratosthenes/) o paralelizaci Eratosthenova síta pomocí [OpenMP](http://www.openmp.org/), včetně zmíněné segmentace.
* [Implementace](http://primesieve.org/segmented_sieve.html) segmentovaného Eratosthenova síta, kterou jsem použil jako referenci k ověření výsledků.