# MI-MCS

Paralelní počítání prvočísel pomocí [Cilk Plus](https://www.cilkplus.org).

## Kompilace

Pokud defaultní GCC (`which gcc`) nepodporuje rozšíření Cilk, je nutné v Makefile specifikovat cestu k Cilk kompilátoru.

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

Dále jsem algoritmus upravil, aby pracoval pouze s lichými čísly (tedy zpracovával pouze liché násobky). Díky tomu se zmenšily paměťové nároky a došlo také ke zrychlení programu. Sekvenční tvorba síta tedy vypadá takto:

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

## Paralelizace

Paralelizaci vytváření síta jsem provedl velmi přímočaře, pouze jsem doplnil `cilk_for` k cyklu, který provádí odstraňování násobků:

```
cilk_for(unsigned int p = i * i; p <= n; p += inc)
            {
                sieve[p / 2] = 0;
            }
```

Paralelizace vnějšího cyklu se nejevila příliš efektivní, pravděpodobně kvůli relativně malému počtu iterací cyklu a nevyvážené časové náročnosti jednotlivých iterací (iterace pro neprvočíselná * i * hned skončí).

`cilk_for` jsem také použil k počáteční inicializaci síta jedničkami. Zde se tento způsob zdál rychlejší než použití [Cilk Array Notatnion](https://www.cilkplus.org/tutorial-array-notation), tedy `sieve[0:sieve_size] = 1` nebo použití funkce [memset](http://www.cplusplus.com/reference/cstring/memset/).





