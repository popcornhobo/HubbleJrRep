----------------------------------------------------------------------------------
-- Company:          Montana State University
-- Author/Engineer:  Zach Frazee
-- 
-- Create Date:    7:14:00 3/13/2016 
-- Design Name: 
-- Module Name: OutputSelect
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

entity OutputSelect is
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
end entity;

architecture OutputSelect_arch of OutputSelect is 

	begin
		
		OutputSelect_Logic : process (sel_line, miso_out, clk_hard, clk_soft, mosi_hard, mosi_soft, ss_soft, ss_hard)
			begin
				--Hardware mode
				if sel_line = '1' then
					clk_out 		<= clk_hard;
					mosi_out 	<= mosi_hard;
					ss_out 		<= ss_soft;		--always use soft slave select for vn-100 api
					
					miso_soft	<= '1';
					miso_hard <= miso_out;
				
				--Software mode
				else
					clk_out 		<= clk_soft;
					mosi_out 	<= mosi_soft;
					ss_out 		<= ss_soft;
					
					miso_soft	<= miso_out;
					miso_hard <= '1';
				end if;	
		end process;
		
end architecture;
	