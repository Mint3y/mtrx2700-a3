
## Group Members & Roles

| Johnathan | Memory Game Developer | Implementation of a modularised memory game that can integrate with Ben's module |

## Challenge Overview

### Memory Game (Johnathan)
The playuer must repeat LED/button sequences across levels with increasing difficulty.

There are 3 levels in total, with varying sequences and light-up frequencies. The
module is capable of running on a single STM32 timer and has a completion callback
that can be set by other modules. The completion callback will run when the player
is successful. Skipping has been implemented for debugging purposes. The module
can be skipped by clicking the blue button 3 times when the debug (yellow) LED is
flashing. The module is started by pressing the blue button.
