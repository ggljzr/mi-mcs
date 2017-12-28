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
# vypocet prvocisel s jednim vlaknem
$ primes -t 1 10000
```

