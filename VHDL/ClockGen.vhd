----------------------------------------------------------------------------------
-- Company:          Montana State University
-- Author/Engineer:  Zach Frazee
-- 
-- Create Date:    7:43:00 3/13/2016 
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

entity ClockGen is
	port
	(
		clk_in		: in std_logic;
		
		reset_in 	: in std_logic;
		
		divisor		: in std_logic_vector(31 downto 0);
		
		clk_out 		: out std_logic
	);
end entity;

architecture ClockGen_arch of ClockGen is
	
	signal count : integer := 0;
	signal clock_int : std_logic := '1';
	
	begin
	
		--Clock modulo counter divider
		Clock_Divider : process(clk_in, reset_in)
			begin
				if(reset_in = '0') then
					count <= 0;
					clock_int <= '1';
				elsif(clk_in'event and clk_in = '1') then
					
					count <= count + 1;
					
					if(count >= to_integer(unsigned(divisor))) then
						clock_int <= not clock_int;
						count <= 0;
					end if;
					
				end if;
		end process;
					
		--Assign clock 
		clk_out <= clock_int;
		
end architecture;