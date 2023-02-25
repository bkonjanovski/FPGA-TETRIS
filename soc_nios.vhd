library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_arith.all;

entity soc_nios is
  port (clk,rst: in std_logic;
	    cols: out std_logic_vector(7 downto 0);
		 rows: out std_logic_vector(11 downto 0);
		 btns: in  std_logic_vector(3 downto 0);
		 
		 SPI_MISO: in  std_logic;
		 SPI_MOSI: out std_logic;
		 SPI_SCLK: out std_logic;
		 SPI_SS_n: out std_logic;
		 SPI_RS:   out std_logic
		 );
		 
		 
 end soc_nios ;

architecture struct of soc_nios is
	component nios_system is
		port (
			clk_clk                          : in  std_logic                    := 'X'; -- clk
			reset_reset_n                    : in  std_logic                    := 'X'; -- reset_n
			pio_2_external_connection_export : in  std_logic_vector(3 downto 0) := (others => 'X')  ;
			pio_0_external_connection_export : out std_logic_vector(7 downto 0) ;   
			pio_1_external_connection_export : out std_logic_vector(11 downto 0);
			
			spi_0_external_MISO              : in  std_logic                     := 'X';             -- MISO
         spi_0_external_MOSI              : out std_logic;                                        -- MOSI
         spi_0_external_SCLK              : out std_logic;                                        -- SCLK
         spi_0_external_SS_n              : out std_logic;                                        -- SS_n
         pio_3_external_connection_export : out std_logic                                         -- export
		);
	end component nios_system;
begin
   	u0 : component nios_system
		port map (
			clk_clk                          => clk,  --                       clk.clk
			reset_reset_n                    => rst,  --                     reset.reset_n
			pio_0_external_connection_export => cols,
			pio_1_external_connection_export => rows,	
		   pio_2_external_connection_export => btns,
			
			spi_0_external_MISO              => SPI_MISO,              --            spi_0_external.MISO
         spi_0_external_MOSI              => SPI_MOSI,              --                          .MOSI
         spi_0_external_SCLK              => SPI_SCLK,              --                          .SCLK
         spi_0_external_SS_n              => SPI_SS_n,              --                          .SS_n
         pio_3_external_connection_export => SPI_RS  -- pio_3_external_connection.export
		);
end struct;