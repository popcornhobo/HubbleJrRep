-- TB.vhd
-- This test bench will stimulate the avalon wrapper directly and was used to test the overall functionality

library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.STD_LOGIC_ARITH.ALL;
use IEEE.STD_LOGIC_UNSIGNED.ALL;

entity TB is
end entity;

architecture TB_arch of TB is

	constant t_clk_per : time := 20 ns;
	
	component SPIWrapper 
		port
		(
			--Avalon Bus
			clk                     		: in std_logic;
			reset_n                 	: in std_logic;
			avs_s1_write            	: in std_logic;
			avs_s1_address         : in std_logic_vector(4 downto 0);
			avs_s1_writedata       : in std_logic_vector(31 downto 0);
			avs_s1_read             	: in std_logic;
			avs_s1_readdata		: out std_logic_vector(31 downto 0);
			
			--Conduits 
			SPI_ss						: out std_logic;
			SPI_clk						: out std_logic;
			SPI_mosi					: out std_logic;
			SPI_miso					: in std_logic
		);
	end component;
	
	--Input Bus Signals
	signal clk_TB						: std_logic;
	signal reset_n_TB					: std_logic;
	signal avs_s1_write_TB			: std_logic;
	signal avs_s1_address_TB		: std_logic_vector(4 downto 0);
	signal avs_s1_writedata_TB	: std_logic_vector(31 downto 0);
	signal avs_s1_read_TB			: std_logic;
	
	--Output Bus Signals
	signal avs_s1_readdata_TB   : std_logic_vector(31 downto 0);
	
	--Conduit Signals
	signal SPI_ss_TB 					: std_logic;
	signal SPI_clk_TB					: std_logic;
	signal SPI_mosi_TB				: std_logic;
	signal SPI_miso_TB 				: std_logic;
	
	begin 
		
		DUT : SPIWrapper
			port map (
				clk                     		=> clk_TB,
				--Avalon Bus
				reset_n                 	=> reset_n_TB,
				avs_s1_write            	=> avs_s1_write_TB,
				avs_s1_address        	=> avs_s1_address_TB,
				avs_s1_writedata       => avs_s1_writedata_TB,
				avs_s1_read             	=> avs_s1_read_TB,
				avs_s1_readdata		=> avs_s1_readdata_TB,
				
				--Conduits 
				SPI_ss						=> SPI_ss_TB,
				SPI_clk						=> SPI_clk_TB,
				SPI_mosi					=> SPI_mosi_TB,
				SPI_miso					=> SPI_miso_TB
			);
			
		CLOCK_STIM : process
			begin
				clk_TB <= '0'; wait for 0.5*t_clk_per; 
				clk_TB <= '1'; wait for 0.5*t_clk_per; 
		end process;
		
		MAIN_STIM : process
			begin
				reset_n_TB <= '0';
				
				avs_s1_address_TB <= "11111";
				avs_s1_writedata_TB <= x"00000000";
				avs_s1_write_TB <= '0';
				avs_s1_read_TB <= '0';
			
				wait for t_clk_per;
			
				reset_n_TB <= '1';
				
				wait for 10 us;
				
				avs_s1_address_TB <= "00000";
				avs_s1_writedata_TB <= x"00000001";
				avs_s1_write_TB <= '1';
				avs_s1_read_TB <= '0';
				
				wait for t_clk_per;
				
				avs_s1_address_TB <= "00001";
				avs_s1_writedata_TB <= x"0000000" & "1101";
				avs_s1_write_TB <= '1';
				avs_s1_read_TB <= '0';
				
				wait for t_clk_per;
				
				avs_s1_address_TB <= "00011";
				avs_s1_writedata_TB <= x"000000F0";
				avs_s1_write_TB <= '1';
				avs_s1_read_TB <= '0';
				
				wait for t_clk_per;
				
				avs_s1_address_TB <= "00101";
				avs_s1_writedata_TB <= x"00000000";
				avs_s1_write_TB <= '0';
				avs_s1_read_TB <= '1';
				
				
				wait for 700 * t_clk_per;
				
				avs_s1_address_TB <= "11111";
				avs_s1_writedata_TB <= x"00000000";
				avs_s1_write_TB <= '0';
				avs_s1_read_TB <= '0';
				
				SPI_miso_TB <= '1';
				
				wait for 200 * t_clk_per;
				
				SPI_miso_TB <= '0';
				
				wait for 2000 * t_clk_per;
				
				avs_s1_address_TB <= "00100";
				avs_s1_writedata_TB <= x"00000000";
				avs_s1_write_TB <= '0';
				avs_s1_read_TB <= '1';
				
				wait for t_clk_per;
				
				avs_s1_address_TB <= "00001";
				avs_s1_writedata_TB <= x"0000000" & "1111";
				avs_s1_write_TB <= '1';
				avs_s1_read_TB <= '0';
				
				
				
				wait for 1000 ms;
		end process;
		
end architecture;