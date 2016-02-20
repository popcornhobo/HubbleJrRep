----------------------------------------------------------------------------------
-- Company:          Montana State University
-- Author/Engineer:  Seth Kreitinger
-- 
-- Create Date:    4:50:00 12/12/2015 
-- Design Name: 
-- Module Name: DynamixelPacketFormer
-- Project Name: 
-- Target Devices: DE2 board
-- Tool versions: 
-- Description: 
--
-- Dependencies: 
--
-- Revision: 
-- Revision 0.01 - File Created
--          0.02 - Packet Former Update Processes
-- Additional Comments: 
--
-- INITIALIZE THE STATE MACHINE!!!!!!!!
----------------------------------------------------------------------------------

library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.STD_LOGIC_ARITH.ALL;
use IEEE.STD_LOGIC_UNSIGNED.ALL;

entity dynamixel_wrapper is
    port 
    (
        clk                     : in std_logic;
        reset_n                 : in std_logic;
        avs_s1_write            : in std_logic;
        avs_s1_address          : in std_logic_vector(4 downto 0);
        avs_s1_writedata        : in std_logic_vector(31 downto 0);
        avs_s1_read             : in std_LOGIC;
        avs_s1_readdata         : out std_logic_vector(31 downto 0);
        Tx_out                  : out std_logic;
        Rx_out                  : in std_logic;
		  TxRx_sig					  : out std_LOGIC
    );
end entity;

architecture dynamixel_wrapper_arch of dynamixel_wrapper is

component packet_former 
    port 
    (
        clk             : in std_logic;
        reset           : in std_logic;
        data_input      : in std_logic_vector(31 downto 0);
        data_output     : out std_logic_vector(7 downto 0);
        ext_status      : out std_logic_vector(31 downto 0);
        servo_error     : out std_logic_vector(31 downto 0);
        read_write      : in std_logic;
        Tx              : out std_logic;
        Rx              : in std_logic;
        TxRx_sel        : out std_logic
    );
end component;


signal wre : std_logic;
signal re : std_logic;
signal addr : std_logic_vector(4 downto 0);


signal Set_rate     : std_logic_vector(31 downto 0);
signal Servo_error  : std_logic_vector(31 downto 0);

signal Status_input : std_logic_vector(31 downto 0);
signal Status_output: std_logic_vector(31 downto 0);

signal data_in_sig : std_logic_vector(31 downto 0);
signal data_out_sig: std_logic_vector(7 downto 0);

signal read_write : std_LOGIC;
signal Tx_sig : std_logic;
signal Rx_sig : std_logic;

begin

wre <= avs_s1_write;
re  <= avs_s1_read;
addr <= avs_s1_address;

Tx_out <= Tx_sig;
Rx_sig <= Rx_out;

SERVO_CONTROLLER: packet_former
    port map
    (
        clk             => clk,
        reset           => reset_n,
        data_input      => Set_rate,
        data_output     => data_out_sig,
        ext_status      => Status_output,
        servo_error     => Servo_error,
        read_write      => read_write,
        Tx              => Tx_sig,
        Rx              => Rx_sig,
        TxRx_sel        => TxRx_sig
    );
    
	 ---------------------------------------------------
	-- Latch Rate word (address 0)
	---------------------------------------------------
	process (clk)
	begin
		if rising_edge(clk) then
			if wre='1' and addr="00000" then 
				Set_rate <= avs_s1_writedata(31 downto 0);
				read_write <= '1';
			else
				read_write <='0';
			end if;
		end if;
	end process;
	 
    REGISTER_READ:process (clk)
    begin
        if rising_edge(clk) and wre = '0' and re = '1' then
            case addr is
                when "00000" =>
                    avs_s1_readdata <= Set_rate;
                when "00001" =>
                    avs_s1_readdata <= x"000000" & data_out_sig;
                when "00010" =>
                    avs_s1_readdata <= Status_output;
                when "00011" =>
                    avs_s1_readdata <= Servo_Error;
                when others =>
                    
            end case;
    end if;
    end process;
	 
end dynamixel_wrapper_arch;
