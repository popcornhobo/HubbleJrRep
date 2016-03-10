----------------------------------------------------------------------------------
-- Company:          Montana State University
-- Author/Engineer:  Seth Kreitinger, Zach Frazee, Tom Rader
-- 
-- Create Date:    4:50:00 12/12/2015 
-- Design Name: 
-- Module Name: DynamixelPacketFormer
-- Project Name: HubbleJr
-- Target Devices: DE0-CycloneV-SOC board
-- Tool versions: 
-- Description: 
--
-- Dependencies: PacketFormer.vhd
--
-- Revision: 
-- Revision 0.10 - File Created
--          0.30 - State Machines updated and Full2half duplex added
--          0.50 - Functional block completed with lock up error on no returned packet
--          0.60 - Alot of Minor Fixes, Excess Signals from prev rev removed
--          0.80 - Added StateMachine Timeout to Prevent Lockup and Set Position States
-- Additional Comments: 
--      For Additional Information see the ReadMe from git repository https://github.com/popcornhobo/HubbleJrRep.git
--
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
        avs_s1_read             : in std_logic;
        avs_s1_readdata         : out std_logic_vector(31 downto 0);
        Tx_out                  : out std_logic;
        Rx_out                  : in std_logic;
		    TxRx_sig			    : out std_logic
    );
end entity;

architecture dynamixel_wrapper_arch of dynamixel_wrapper is

component packet_former 
    port 
    (
        clk             : in std_logic;
        reset           : in std_logic;
        data_input      : in std_logic_vector(31 downto 0);
        ext_status      : out std_logic_vector(31 downto 0);
        servo_error     : out std_logic_vector(31 downto 0);
        reg_id          : in std_logic_vector(4 downto 0);
        Tx              : out std_logic;
        Rx              : in std_logic;
        TxRx_sel        : out std_logic
    );
end component;

-- Read enable and write enable Avalon signals
signal wre : std_logic;
signal re : std_logic;
-- Avalon requested address
signal addr : std_logic_vector(4 downto 0);

-- Register storage signals
signal Set_Pos      : std_logic_vector(31 downto 0);
signal Set_rate     : std_logic_vector(31 downto 0);
signal Servo_error  : std_logic_vector(31 downto 0);
signal Block_Reset  : std_logic_vector(31 downto 0);
signal Set_CCW		  : std_logic_vector(31 downto 0);
signal Set_CW		  : std_logic_vector(31 downto 0);

-- Packet former status byte
signal Status_output: std_logic_vector(31 downto 0);

-- The user supplied data from the Avalon Bus
signal data_in_sig: std_logic_vector(31 downto 0);

-- reg_id is used to determine which register was written to. It is one hot encoded.
-- 0001 is SetRate
-- 0010 is SetPos
-- 0100 is BlockReset
signal reg_id : std_logic_vector(4 downto 0) := "00100";

-- Tx and Rx signal from top-level entity
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
        data_input      => data_in_sig,
        ext_status      => Status_output,
        servo_error     => Servo_error,
        reg_id          => reg_id,
        Tx              => Tx_sig,
        Rx              => Rx_sig,
        TxRx_sel        => TxRx_sig
    );
    
	 ---------------------------------------------------
	-- Latch Rate word (address 0)
	---------------------------------------------------
	REGISTER_WRITE:process (clk)
	begin
		if rising_edge(clk) then
			if wre = '1' then 
            case addr is
                when "00000" =>
                    data_in_sig <= avs_s1_writedata(31 downto 0);
						  Set_rate <= avs_s1_writedata(31 downto 0);
                    reg_id <= "00101";
                when "00011" =>
                    data_in_sig <= avs_s1_writedata(31 downto 0);
						  Set_Pos <= avs_s1_writedata(31 downto 0);
                    reg_id <= "00110";
                when "00100" =>
						  Block_Reset <= avs_s1_writedata(31 downto 0);
                    reg_id <= "00000";
					 when "00101" =>
						  data_in_sig <= avs_s1_writedata(31 downto 0);
						  Set_CCW <= avs_s1_writedata(31 downto 0);
						  reg_id <= "01100";
					 when "00110" =>
						  data_in_sig <= avs_s1_writedata(31 downto 0);
						  Set_CW <= avs_s1_writedata(31 downto 0);
						  reg_id <= "10100";
					 when others =>
						  reg_id <= "00100";
            end case;
			else
				reg_id <= "00100";
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
                    avs_s1_readdata <= Status_output;
                when "00010" =>
                    avs_s1_readdata <= Servo_Error;
                when "00011" =>
                    avs_s1_readdata <= Set_Pos;
                when "00100" =>
                    avs_s1_readdata <= Block_Reset;
					 when "00101" =>
						  avs_s1_readdata <= Set_CCW;
					 when "00110" =>
						  avs_s1_readdata <= Set_CW;
                when others =>
                    
            end case;
        end if;
    end process;
	 
end dynamixel_wrapper_arch;
