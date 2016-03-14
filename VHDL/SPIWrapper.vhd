----------------------------------------------------------------------------------
-- Company:          Montana State University
-- Author/Engineer:  Zach Frazee
-- 
-- Create Date:    12:11:00 3/14/2016 
-- Design Name: 
-- Module Name: ClockGen
-- Project Name: HubbleJr
-- Target Devices: DE0-CycloneV-SOC board
-- Tool versions: 
-- Description: 
--
-- Dependencies: PacketFormer.vhd
--
-- Revision: 
-- Revision 0.10 - File Created
-- Additional Comments: 
--      For Additional Information see the ReadMe from git repository https://github.com/popcornhobo/HubbleJrRep.git
--
----------------------------------------------------------------------------------

library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.ALL;
use IEEE.STD_LOGIC_UNSIGNED.ALL;

entity SPIWrapper is 
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
end entity;


architecture SPIWrapper_arch of SPIWrapper is
		
		-----------------------------------------------------
		--COMPONENTS
		-----------------------------------------------------
		
		component ClockGen
			port
			(
				clk_in		: in std_logic;			
				reset_in 	: in std_logic;				
				divisor		: in std_logic_vector(31 downto 0);	
				clk_out 		: out std_logic
			);
		end component;
		
		component OutputSelect
			port
			(
				clk_out 			: out std_logic;
				mosi_out 		: out std_logic;
				miso_out 		: in std_logic;
				ss_out			: out std_logic;
				
				clk_hard 		: in std_logic; 
				mosi_hard	 	: in std_logic;
				miso_hard 	: out std_logic;
				ss_hard			: in std_logic;
				
				clk_soft 		: in std_logic;
				mosi_soft  	: in std_logic;
				miso_soft  	: out std_logic;
				ss_soft 			: in std_logic;
				
				sel_line			: in std_logic
			);
		end component;
		
		component spi_abstract
			generic (
				cpol_cpha : std_logic_vector(1 downto 0) := "00"
			);
			port(
				clk	  						: in std_logic;	
				rst_n 	    				: in std_logic;	

				mosi_data_i         		: in std_logic_vector(7 downto 0);
				miso_data_o         		: out std_logic_vector(7 downto 0);
				mosi_data_valid_i 		: in std_logic;	
				mosi_data_ack_o 	  	: out std_logic;	
				miso_data_valid_o		: out std_logic;	

				miso 						: in std_logic;	
				mosi 						: out std_logic;	
				sclk 							: out std_logic;	
				cs_n 						: out std_logic		 
			);
		end component;
		
		component SignalLatch
			port
			(
				clk_in		: in std_logic;
				reset_in 	: in std_logic;
				hold			: in std_logic;
				hold_time  : in std_logic_vector(31 downto 0);
				sig_out 		: out std_logic
			);
		end component;
		
		-----------------------------------------------------
		--SIGNALS
		-----------------------------------------------------
		
		--REGISTERS
		signal reg_output_mode : std_logic := '0';
		signal reg_direct_output : std_logic_vector (3 downto 0) := "1111";
		signal reg_divisor : std_logic_vector (31 downto 0) := x"00000064";
		signal reg_hard_spi_input : std_logic_vector (7 downto 0) := x"00";
		signal reg_hard_spi_output : std_logic_vector (7 downto 0);
		signal reg_hard_spi_status : std_logic;
		
		--SPI Internal Output Signals
		signal spi_clock_int : std_logic;
		signal spi_ss_int : std_logic;
		signal spi_mosi_int : std_logic;
		signal spi_miso_int : std_logic;
		
		--SPI Internal Hardware Signals
		signal spi_hard_clock_int : std_logic;
		signal spi_generated_clock_int : std_logic;
		
		signal spi_hard_ss_int : std_logic;
		signal spi_hard_mosi_int : std_logic;
		signal spi_hard_miso_int : std_logic;
		
		signal spi_hard_output : std_logic_vector (7 downto 0); 
		signal spi_hard_ack : std_logic;
		signal spi_hard_valid_out : std_logic;
		
		signal spi_hard_data_valid : std_logic;
		signal spi_hard_data_valid_latched : std_logic;
		signal data_valid_hold_time : std_logic_vector(31 downto 0);
		signal data_valid_hold_time_large : std_logic_vector(63 downto 0);
		
		-----------------------------------------------------
		--BEGIN
		-----------------------------------------------------
		
		begin
			
			-----------------------------------------------------
			--COMPONENT INSTANTIATION
			-----------------------------------------------------
			
			CLOCK_GENERATOR : ClockGen
			port map
			(
				clk_in 		=> clk,
				reset_in 	=> reset_n,
				divisor 		=> reg_divisor,
				clk_out 		=> spi_generated_clock_int
			);
			
			OUTPUT_ROUTER : OutputSelect
			port map
			(
				clk_out 			=> spi_clock_int,
				mosi_out		=> spi_mosi_int,
				miso_out		=> spi_miso_int,
				ss_out			=> spi_ss_int,
				
				clk_hard		=> spi_hard_clock_int,
				mosi_hard		=> spi_hard_mosi_int,
				miso_hard		=> spi_hard_miso_int,
				ss_hard			=> spi_hard_ss_int,
				
				clk_soft			=> reg_direct_output(0),
				mosi_soft		=> reg_direct_output(3),
				miso_soft		=> reg_direct_output(2),
				ss_soft			=> reg_direct_output(1),
				
				sel_line			=> reg_output_mode
			);
			
			SPI_BLOCK : spi_abstract
			generic map
			(
				cpol_cpha 					=> "11"
			)
			port map
			(
				clk 							=> spi_generated_clock_int,
				rst_n							=> reset_n,

				mosi_data_i				=> reg_hard_spi_input,
				miso_data_o				=> spi_hard_output,
				mosi_data_valid_i		=> spi_hard_data_valid_latched,
				mosi_data_ack_o		=> spi_hard_ack,
				miso_data_valid_o		=> spi_hard_valid_out,

				miso							=> spi_hard_miso_int,
				mosi							=> spi_hard_mosi_int,
				sclk							=> spi_hard_clock_int,
				cs_n							=> spi_hard_ss_int
			);
			
			DATA_VALID_LATCH : SignalLatch
			port map
			(
				clk_in		=> clk,
				
				reset_in 	=> reset_n,
				
				hold			=> spi_hard_data_valid,
				
				hold_time  => data_valid_hold_time,
				
				sig_out 		=> spi_hard_data_valid_latched
			);
			
			-----------------------------------------------------
			--SIGNAL ASSIGNMENTS
			-----------------------------------------------------
			
			SPI_ss <= spi_ss_int;
			SPI_clk <= spi_clock_int;
			SPI_mosi <= spi_mosi_int;
			spi_miso_int <= SPI_miso;
			reg_hard_spi_status <= '1' when (spi_hard_ss_int = '1' and spi_hard_data_valid_latched = '0') else '0';
			
			data_valid_hold_time_large <= std_logic_vector((unsigned(reg_divisor) * 5) + 4);
			data_valid_hold_time <= data_valid_hold_time_large(31 downto 0);
			
			-----------------------------------------------------
			--REGISTER WRITE
			-----------------------------------------------------
			REGISTER_WRITE : process (clk)
				begin
					if(clk'event and clk = '1') then
						if(avs_s1_write = '1') then
							case avs_s1_address is
								--OUTPUT MODE
								when "00000" =>
									reg_output_mode 		<= avs_s1_writedata(0);
									spi_hard_data_valid	<= '0';
								
								--DIRECT OUTPUT
								when "00001" =>
									reg_direct_output(0) 	<= avs_s1_writedata(0);
									reg_direct_output(1) 	<= avs_s1_writedata(1);
									reg_direct_output(3) 	<= avs_s1_writedata(3);
									spi_hard_data_valid	<= '0';
								
								--CLOCK DIVISOR
								when "00010" =>
									reg_divisor 			 	<= avs_s1_writedata(31 downto 0); 
									spi_hard_data_valid	<= '0';
								
								--SPI INPUT BYTE
								when "00011" =>
									reg_hard_spi_input	<= avs_s1_writedata(7 downto 0); 
									spi_hard_data_valid	<= '1';
								
								--SPI OUTPUT BYTE
								when "00100" =>
									--DO NOTHING - Read only register
									spi_hard_data_valid	<= '0';
									
								--SPI STATUS
								when "00101" =>
									--DO NOTHING - Read only register
									spi_hard_data_valid	<= '0';
									
								--OTHERS
								when others  =>
									--INVALID ADDRESS
									spi_hard_data_valid	<= '0';
									
							end case;
						else
							spi_hard_data_valid	<= '0';
						end if;
					end if;
			end process;
			
			-----------------------------------------------------
			--REGISTER READ
			-----------------------------------------------------
			REGISTER_READ : process (clk)
				begin
					if (clk'event and clk = '1') and avs_s1_write = '0' and avs_s1_read = '1' then
						case avs_s1_address is
							--OUTPUT MODE
							when "00000" =>
								avs_s1_readdata <= x"0000000" & "000" & reg_output_mode;
								
							--DIRECT OUTPUT
							when "00001" =>
								avs_s1_readdata <= x"0000000" & reg_direct_output;
							
							--CLOCK DIVISOR
							when "00010" =>
								avs_s1_readdata <= reg_divisor;
							
							--SPI INPUT BYTE
							when "00011" =>
								avs_s1_readdata <= x"000000" & reg_hard_spi_input;
							
							--SPI OUTPUT BYTE
							when "00100" =>
								avs_s1_readdata <= x"000000" & reg_hard_spi_output;
							
							--SPI STATUS
							when "00101" =>
								avs_s1_readdata <= x"0000000" & "000" & reg_hard_spi_status;
							
							--OTHERS
							when others =>
								
						end case;
					end if;
			end process;
			
			-----------------------------------------------------
			--OUTPUT DATA LATCH
			-----------------------------------------------------
			OUTPUT_LATCH : process (clk, reset_n)
				begin
					if(reset_n = '0') then
						reg_hard_spi_output <= x"00";
					elsif(rising_edge(clk))then
						if(spi_hard_valid_out = '1') then
							reg_hard_spi_output <= spi_hard_output;
						end if;
					end if;
			end process;
			
			
end architecture;