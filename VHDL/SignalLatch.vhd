----------------------------------------------------------------------------------
-- Company:          Montana State University
-- Author/Engineer:  Zach Frazee
-- 
-- Create Date:    3:30:00 3/14/2016 
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

entity SignalLatch is
	port
	(
		clk_in		: in std_logic;
		
		reset_in 	: in std_logic;
		
		hold			: in std_logic;
		
		hold_time  : in std_logic_vector(31 downto 0);
		
		sig_out 		: out std_logic
	);
end entity;

architecture SignalLatch_arch of SignalLatch is
	
	signal count : integer := 0;

	begin 
		
		--Signal Timer
		Timer : process(clk_in, reset_in)
			begin
				if(reset_in = '0') then
					count <= 0;
				elsif(hold = '1') then
					count <= 1;
				elsif(clk_in'event and clk_in = '1') then
					
					if(count > 0) then 
						count <= count + 1;
					end if;
					
					if(count >= to_integer(unsigned(hold_time))) then
						count <= 0;
					end if;
					
				end if;
		end process;
		
		Output_Logic : process(count)
			begin
				if(count = 0) then
					sig_out <= '0';
				else
					sig_out <= '1';
				end if;
		end process;
		
end architecture;