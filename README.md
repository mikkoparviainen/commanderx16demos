# Commander X16 demo programs

Author: Mikko Parviainen (mikko.parviainen@iki.fi)

In this repository there are demo programs I have written for the
[Commander X16](https://github.com/commanderx16). That project is not mine,
but I'm interested in it. 

## Demos

There is only one demo at the moment. They are written for the 
[ACME compiler](https://github.com/meonwax/acme) but can probably be reasonably easily
ported to other ones. I have written these using the CX16 320x240 8bpp mode (mode 7), because
it has the easiest pixels to use. Modifying the code to work in other modes is
of course possible and a good learning experience.

The demos are meant to be examples of how to program the CX16 and not complete
useful programs.


### Pixel drawing demo

This demo initializes the VERA graphics mode, clears the screen, and then draws pixels
at random positions with random colors. Currently there is no way to stop the demo,
so reset the emulator (or boot the hardware) if you need to stop it.

The function `vera_init_320x240_8bpp` does what its name says. It initializes the
registers and sets the mode.

The graphics functions use certain locations in memory to pass the parameters. I
considered using something else, but this was simple enough and especially the
pixel drawing routine needs to be fast. The global `colour` location is the drawing
color for all the functions. Do not assume anything from the VERA registers
after calling the functions.

The function `clear_screen` should be simple to understand. The 320x240 8bpp mode
is a bunch of pixels and the function just writes those 320x240 pixels with the 
set color. It needs to do the double looping twice, because 320x240 is more than
255x255, and so can't be looped using the `X` and `Y` registers once. It uses
the VERA functionality for increasing the index after each write, so the meat of the
loop is just the `sta veradat`.

The function `draw_pixel` draws a pixel in the coordinates given in the memory
locations `x` and `y`. These are 16-bit values, because in this mode at least
the X coordinate can be more than 255, and to be more future proof the Y coordinate
is treated as such. This function assumes a useful input of less than 512 for
each coordinate and it ignores the bits 1-7 of the upper bytes of the parameters.

The calculation of the correct graphics memory location is not very clear from the code.
The basic premise is that the linear position in the memory is calculated with

```
MEMPOS = 320 * Y + X
```

This calculation is complicated by the fact that the 6502 does not have a multiplication
instruction. However, it does have the shift operations, which either multiply or
divide by two. The result memory position has to be three bytes long, because the
graphics memory is larger than 64 KB. This calculates the memory position with 

```
MEMPOS = (256 + 64) * Y + X = 256 * Y + 64 * Y + X
```

(256 + 64 = 320) The multiplication of Y by 256 is easily done: just move the two
bytes of the Y coordinate into the mid and high bytes of the result. The multiplication
by 64 is done in two parts. First the low byte of the Y coordinate is shifted left
six times and it's moved to the low byte of the result. At this point the low byte
is zero, so no addition is needed.

Secondly, the low byte of the Y coordinate is re-loaded into the accumulator and
shifted right twice. This results in the upper byte of the `Y*64` result and is
the same as grabbing the left shifted bytes in the previous step would have been.
In my view this is simpler. This byte is then added to the mid byte of the result
and the carry is carried on to the high byte. 

Then the same operation is done for the higher byte of the Y coordinate, but because
we assume that it's either `$00` or `$01`, the upper bytes are not considered.

After this the result contains `MEMPOS = 320 * Y`. Then we just add the two-byte
X coordinate to the result to get the final memory position using the regular addition
algorithm.

Finally the result coordinate is sent to VERA and then a color byte written there.

The random number generator is not by me, see the license section.

The demo still does not have any way to stop it, so reset the computer to stop it. It
also does not reset the graphics mode. The random number generator is slow, but
might represent the game logic in a possible game.

I am sure the routines can be optimized and I'm happy to listen to improvements.


## Includes

There is one include file at the moment. The include files are in the `includes`
directory.

### `system.inc`

This include file contains the macro `basic_sys` and the pseudo random functions
`getrandom_bounded` and `getrandom`.

The macro `basic_sys` just inserts the line "1 SYS2061" into the code. This is
meant to be used after `*=$0801` so the program can be run with just `RUN`.

The function `getrandom` calculates sixteen bits of random number into the bytes
`random` and `random+1`.

The function `getrandom_bounded` gets the 16-bit bound from the memory locations
`bound` and `bound+1`. It then gets a random number from the `getrandom` and
sees if the number is less than the bound. If not, it tries again. For purposes
of these demos, the random numbers are assumed to be less than 512, so the function
masks away the higher bits. This is to make it faster to get random numbers -
trying to get a number less than 320 when randomizing all the 65536 values would
take too much time.

The randomness of the bits is not that high, though, but it's sufficient for demo
purposes.

## Licensing

The code in this repository is mostly mine. The random number generator
in the code is from the [Codebase 64](https://codebase64.org) and it's page
is [this](base:two_very_fast_16bit_pseudo_random_generators_as_lfsr). The license
of that is LGPL, so if I'm conservative, everything here is also LPGL. I'd argue though
that using it in this context is using it as a library, so my code wouldn't be LPGL.
This repository is meant to be more of a demo, and I'm not sure of the commercial 
possibilities of the CX16, but if you use my code I'd appreciate at least a mention.

Some portions of the code are taken from the CX16 repository, like the start which
lets users just type 'RUN' to run the code.