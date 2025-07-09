# RPIPico2TST-Exp

This is a demonstration of TST-Center from [www.truesmarttech.com](www.truesmarttech.com).  Made for an episode of my [YouTube channel](https://youtube.com/@drjonea).

*LICENCE*: Although my code and FreeRTOS Kernel used in this example is under MIT licence 
the library from www.truesmarttech.com is commercial and should only be used with their specific approval. The PI_SPIGOT library is licence under boost.

## Examples

I've included two examples with TST-Center code to expose variables. 

+ 2CoreRTOS: Calculates the value of PI to 1,000 decimal places for 1 minute. Count how many times this is achieved by each core.
+ FreeRTOSMetrics: Expose some FreeRTOSMetrics such as heap space and Task count to TST-Center

## Clone and Build
Submodules are used so please close with the *--recurse-submodules* flag.

In each of the two examples:
```
mkdir build
cd build
cmake ..
make
```

the binaries (UF2 and ELF) will be in the *build/src* folder.