# FPGA-TETRIS
This is a practical Hardware\Software implementation of a classic TETRIS game using an <a href="https://www.terasic.com.tw/cgi-bin/page/archive.pl?Language=English&CategoryNo=165&No=1061&PartNo=3#contents">Intel ADC-SoC Board</a>. <br />
The board is based on <a href="https://www.intel.com/content/www/us/en/products/sku/210457/cyclone-v-5csea4-fpga/specifications.html">Altera Cyclone V FPGA</a> and contains all necessary components\buses. <br />
A separate custom board was used for I/O, with 4 push buttons, a 8x12 LED Matrix and a LCD display. 

![IO_FPGA](./IO_FPGA.jpg)

The FPGA Board is connected with this custom board via a 40-pin GPIO expansion header. <br>
The <a href="https://www.intel.com/content/www/us/en/products/details/fpga/development-tools/quartus-prime.html">Quartus Prime Design Software</a> was used for development and the designs with the source code were flashed by USB.
