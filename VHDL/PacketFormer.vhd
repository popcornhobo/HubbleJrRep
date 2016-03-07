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
--          0.02 - State Machines updated and Full2half duplex added
-- Additional Comments: 
--
----------------------------------------------------------------------------------

library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.ALL;

entity packet_former is
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
end entity;


architecture packet_former_arch of packet_former is

    -- Build an enumerated type for the state machine
    type Pformer_state is (S0,                                                         -- Default waiting state                                      
									SWR0, SWR_CALC_CHECKSUM, SWR_B1, SWR_B2, SWR_B3,            -- Write the Rotation Rate        
											SWR_B4, SWR_B5, SWR_B6, SWR_B7, SWR_BUF, SWR_B9,
											SWR_B8, SWR_B10,
									SRE0, SRE_B1, SRE_B2, SRE_B3, SRE_B4, SRE_B5, SRE_B6, SRE_B7, SRE_B8, SRE_B9, SRE_B10, SRE_B11, SRE_B12, SRE_ERR, SRE_ERR2,                                                           -- Read the returned error packet after a write
									SIU0, SIU_LCR1, SIU_LCR2, SIU_LCR3, SIU_BS1, SIU_BS2, SIU_BS3, SIU_BS4, SIU_IRQ, SIU_IRQ1, SIU_FIFO, SIU_FIFO1, SIU_FIFO2, SIU_INT);   -- Intialize the UART by setting baud rate registers, the interupt registers, and the FIFO config

 -- Registers to hold the current state and the next state
    signal cur_state, next_state       : Pformer_state;
    signal UART_init    : std_logic := '0';
    signal addr : std_logic_vector(2 downto 0);
    signal dataIn, dataOut : std_logic_vector(7 downto 0);
    signal chipSelect: std_logic;
    signal readWrite: std_logic;
    signal clrSend, dataRdy, dataCarrierDetect, dataTermReady, requestSend, Tdma, Rdma, IRQ, baudClk, ringIndicator : std_logic;
    signal usrOut1, usrOut2 : std_logic;
    signal calculatedBaudByte : unsigned(31 downto 0);
    signal calculatedChecksum : std_logic_vector(7 downto 0);
    signal Ext_Status_Sig: std_logic_vector(31 downto 0);
    signal Tx_sig, Rx_sig : std_logic;
    signal edgeCount : integer := 0;
	 signal FIFO_byte_count : integer := 0;
	 signal servo_error_latched : std_logic_vector (31 downto 0);
	 signal write_error : std_logic;
    signal servo_error_temp : std_logic_vector (31 downto 0);
	 
    component gh_uart_16550
    port(
        clk     : in std_logic;
        BR_clk  : in std_logic;
        rst     : in std_logic;
        CS      : in std_logic;
        WR      : in std_logic;
        ADD     : in std_logic_vector(2 downto 0);
        D       : in std_logic_vector(7 downto 0);
        
        sRX     : in std_logic;
        CTSn    : in std_logic := '1';
        DSRn    : in std_logic := '1';
        RIn     : in std_logic := '1';
        DCDn    : in std_logic := '1';
        
        sTX     : out std_logic;
        DTRn    : out std_logic;
        RTSn    : out std_logic;
        OUT1n   : out std_logic;
        OUT2n   : out std_logic;
        TXRDYn  : out std_logic;
        RXRDYn  : out std_logic;
        
        IRQ     : out std_logic;
        B_CLK   : out std_logic;
        RD      : out std_logic_vector(7 downto 0)
        );
    end component;

	signal n_reset: std_logic;
	
begin

	n_reset <= not (reset); 

    ext_status <= Ext_Status_Sig; -- ??? This should be okay?
	 
    UART_BLOCK: gh_uart_16550
        port map(
            clk     => clk,
            BR_clk  => clk,
            rst     => n_reset,
            CS      => chipSelect,
            WR      => readWrite,
            ADD     => addr,
            D       => dataIn,
            
            sRX     => Rx,
            CTSn    => clrSend,
            DSRn    => dataRdy,
            RIn     => ringIndicator,
            DCDn    => dataCarrierDetect,
            
            sTX     => Tx,
            DTRn    => dataTermReady,
            RTSn    => requestSend,
            OUT1n   => usrOut1,
            OUT2n   => usrOut2,
            TXRDYn  => Tdma,
            RXRDYn  => Rdma,
            
            IRQ     => IRQ,
            B_CLK   => baudClk,
            RD      => dataOut
            );
	 
    OUTPUT_LOGIC: process (cur_state)
    begin
        case cur_state is
            when S0 =>      -- defualt state
            -- Go into Rx mode
            TxRx_sel <= '1';    -- 0 is Rx
            readWrite <= '0';
				chipSelect <= '1';
				write_error <= '0';
				
            -- Set Write Ready Flag: STATUS(2)
            ext_Status_Sig <= ext_Status_Sig or x"00000004";		-- set the WriteReady Flag leave other flags alone
            
            --------------------------------------------------
            -- Setup UART block
            --------------------------------------------------
            when SIU0 =>
                UART_init <= '1';
                readWrite <= '0';
                chipSelect <= '1';
                addr <= "011";        -- LCR reg
                dataIn <= "10000011"; --0x83
					 -- Clear Write Ready Flag: STATUS(2)
                ext_status_sig <= ext_status_sig and x"FFFFFFFA"; 	-- clear ReadReady and WriteReady
				
            when SIU_LCR1 =>
                readWrite <= '1';
					 addr <= "011"; 
                dataIn <= "10000011"; --0x83
					 chipSelect <= '1';
					 -- Clear Write Ready Flag: STATUS(2)
                ext_status_sig <= ext_status_sig and x"FFFFFFFA";		-- clear ReadReady and WriteReady
					 
            when SIU_BS1 =>
                readWrite <= '0';
                addr <= "000";
                dataIn <= x"1B";
					 chipSelect <= '1';
					 -- Clear Write Ready Flag: STATUS(2)
                ext_status_sig <= ext_status_sig and x"FFFFFFFA";		-- clear ReadReady and WriteReady
					 
            when SIU_BS2 =>
					 addr <= "000";
                readWrite <= '1';
					 dataIn <= x"1B";
					 chipSelect <= '1';
					 -- Clear Write Ready Flag: STATUS(2)
                ext_status_sig <= ext_status_sig and x"FFFFFFFA";		-- clear ReadReady and WriteReady
                
            when SIU_BS3 =>
                readWrite <= '0';
                addr <= "001";    
                dataIn <= x"00";
					 chipSelect <= '1';
					 -- Clear Write Ready Flag: STATUS(2)
                ext_status_sig <= ext_status_sig and x"FFFFFFFA";		-- clear ReadReady and WriteReady
				
            when SIU_BS4 =>
                readWrite <= '1';
                dataIn <= x"00";
					 chipSelect <= '1';
					 addr <= "001";    
					 -- Clear Write Ready Flag: STATUS(2)
                ext_status_sig <= ext_status_sig and x"FFFFFFFA";		-- clear ReadReady and WriteReady
					 
            -- Re-Initialize LCR bit 7 to '1' after baud set
            when SIU_LCR2 =>
                readWrite <= '0';
                addr <= "011";        -- LCR reg
                dataIn <= "00000011";
					 chipSelect <= '1';
					 -- Clear Write Ready Flag: STATUS(2)
                ext_status_sig <= ext_status_sig and x"FFFFFFFA";		-- clear ReadReady and WriteReady
					 
            when SIU_LCR3 =>
                readWrite <= '1';
					 dataIn <= "00000011";
					 chipSelect <= '1';
					 addr <= "011";  
					 -- Clear Write Ready Flag: STATUS(2)
                ext_status_sig <= ext_status_sig and x"FFFFFFFA";		-- clear ReadReady and WriteReady
                
            when SIU_IRQ => 
                readWrite <= '0';
                addr   <= "001";        
                dataIn <= x"03";
					 chipSelect <= '1';
					 -- Clear Write Ready Flag: STATUS(2)
                ext_status_sig <= ext_status_sig and x"FFFFFFFA";		-- clear ReadReady and WriteReady
					 
            when SIU_IRQ1 =>
                readWrite <= '1';
					 dataIn <= x"03";
					 chipSelect <= '1';
					 addr   <= "001";
					 -- Clear Write Ready Flag: STATUS(2)
                ext_status_sig <= ext_status_sig and x"FFFFFFFA";		-- clear ReadReady and WriteReady
                
            -- set FIFO trigger level in FCR register     
            when SIU_FIFO =>
                readWrite <= '0';
                addr <= "010";
                dataIn <= "01000000";
					 chipSelect <= '1';
					 -- Clear Write Ready Flag: STATUS(2)
                ext_status_sig <= ext_status_sig and x"FFFFFFFA";		-- clear ReadReady and WriteReady
					 
            when SIU_FIFO1 =>
                readWrite <= '1';
					 dataIn <= "01000000";
					 chipSelect <= '1';
					 addr <= "010";
					 -- Clear Write Ready Flag: STATUS(2)
                ext_status_sig <= ext_status_sig and x"FFFFFFFA";		-- clear ReadReady and WriteReady
					 
				when SIU_FIFO2 =>
					readWrite <= '0';
					chipSelect <= '0';
					-- Clear Write Ready Flag: STATUS(2)
                ext_status_sig <= ext_status_sig and x"FFFFFFFA";		-- clear ReadReady and WriteReady
					 
            --------------------------------------------------
            -- Write Rate States
            --------------------------------------------------
            when SWR0 =>
                TxRx_sel <= '1';    -- 1 is tx
                -- Clear Write Ready Flag: STATUS(2)
                ext_status_sig <= ext_status_sig and x"FFFFFFFA"; -- clear ReadReady and WriteReady
                
             when SWR_CALC_CHECKSUM =>   
                -- calculate checksum
					      TxRx_sel <= '1';    -- 1 is tx
                -- Checksum = (~(ID + AX_BD_LENGTH + AX_WRITE_DATA + AX_BAUD_RATE + Baud_Rate))&0xFF;
                calculatedChecksum <= not(std_logic_vector(to_unsigned(( 1 + 5 + 3 + 32 + to_integer(unsigned(data_input(7 downto 0))) + to_integer(unsigned(data_input(15 downto 8)))),8))); -- 4 is MX_BD_Length, 3 is MX_Write_data 
                -- Clear Write Ready Flag: STATUS(2)
                ext_status_sig <= ext_status_sig and x"FFFFFFFA";		-- clear ReadReady and WriteReady
					 
            when SWR_B1 =>
                -- clock in first byte to the FIFO buffer
                TxRx_sel <= '1';
                dataIn <= x"FF";
                addr <= "000"; -- Select the FIFO buffers 
                readWrite <= '1';
                chipSelect <= '1';               
					 -- Clear Write Ready Flag: STATUS(2)
                ext_status_sig <= ext_status_sig and x"FFFFFFFA";		-- clear ReadReady and WriteReady
					 
            when SWR_B2 =>
                 -- clock in second byte to the FIFO buffer
					 TxRx_sel <= '1';    -- 1 is tx
                dataIn <= x"FF";
					 addr <= "000"; -- Select the FIFO buffers 
                readWrite <= '1';
                chipSelect <= '1';     
					 -- Clear Write Ready Flag: STATUS(2)
                ext_status_sig <= ext_status_sig and x"FFFFFFFA";		-- clear ReadReady and WriteReady
					 
            when SWR_B3 =>
                -- Servo ID
					 TxRx_sel <= '1';    -- 1 is tx
                dataIn <= x"01"; --servo_id(7 downto 0);
					 addr <= "000"; -- Select the FIFO buffers 
                readWrite <= '1';
                chipSelect <= '1';     
					 -- Clear Write Ready Flag: STATUS(2)
                ext_status_sig <= ext_status_sig and x"FFFFFFFA";		-- clear ReadReady and WriteReady
    
            when SWR_B4 =>
                -- Data_length
					 TxRx_sel <= '1';    -- 1 is tx
                dataIn <= x"05"; -- This is the data length
					 addr <= "000"; -- Select the FIFO buffers 
                readWrite <= '1';
                chipSelect <= '1';     
					 -- Clear Write Ready Flag: STATUS(2)
                ext_status_sig <= ext_status_sig and x"FFFFFFFA";		-- clear ReadReady and WriteReady

            when SWR_B5 =>
                -- Write_Data
					 TxRx_sel <= '1';    -- 1 is tx
                dataIn <= x"03"; -- This signifies we are writing data
					 addr <= "000"; -- Select the FIFO buffers 
                readWrite <= '1';
                chipSelect <= '1';   
					 -- Clear Write Ready Flag: STATUS(2)
                ext_status_sig <= ext_status_sig and x"FFFFFFFA";	 	-- clear ReadReady and WriteReady
            
            when SWR_B6 =>
                -- Send Address
					 TxRx_sel <= '1';    -- 1 is tx
                dataIn <= x"20"; --std_logic_vector(calculatedBaudByte)(7 downto 0);
					 addr <= "000"; -- Select the FIFO buffers 
                readWrite <= '1';
                chipSelect <= '1';   
					 -- Clear Write Ready Flag: STATUS(2)
                ext_status_sig <= ext_status_sig and x"FFFFFFFA";	 		-- clear ReadReady and WriteReady
            
            when SWR_B7 =>
                -- Send 1st byte
					 TxRx_sel <= '1';    -- 1 is tx
                dataIn <= data_input(7 downto 0);
					 addr <= "000"; -- Select the FIFO buffers 
                readWrite <= '1';
                chipSelect <= '1';    
					 -- Clear Write Ready Flag: STATUS(2)
                ext_status_sig <= ext_status_sig and x"FFFFFFFA"; 		-- clear ReadReady and WriteReady
                
				when SWR_B8 =>
					 -- Send 2nd byte
					 TxRx_sel <= '1';    -- 1 is tx
                dataIn <= data_input(15 downto 8);
					 addr <= "000"; -- Select the FIFO buffers 
                readWrite <= '1';
                chipSelect <= '1';    
					 -- Clear Write Ready Flag: STATUS(2)
                ext_status_sig <= ext_status_sig and x"FFFFFFFA"; 		-- clear ReadReady and WriteReady
					 
				when SWR_B9 =>
					 -- Send Checksum byte
					 TxRx_sel <= '1';    -- 1 is tx
                dataIn <= calculatedChecksum;
					 addr <= "000"; -- Select the FIFO buffers 
                readWrite <= '1';
                chipSelect <= '1';    
					 -- Clear Write Ready Flag: STATUS(2)
                ext_status_sig <= ext_status_sig and x"FFFFFFFA"; 		-- clear ReadReady and WriteReady
					 
            when SWR_B10 =>
                -- stop writing
					 TxRx_sel <= '1';    -- 1 is tx
                readWrite <= '0';
                chipSelect <= '0';
					 addr <= "000"; -- Select the FIFO buffers 
					 -- Clear Write Ready Flag: STATUS(2)
                ext_status_sig <= ext_status_sig and x"FFFFFFFA";		-- clear ReadReady and WriteReady
					 
				----------------------------------------------------------------------
				-- Read Servo Returned Error
				----------------------------------------------------------------------
					 
				when SRE0 =>		-- RESET the RX FIFO and FIFO byte count
				  TxRx_sel <= '0';
				  addr <= "000";									
					readWrite <= '0';
					chipSelect <= '0';
					write_error <= '0';
                -- Clear Read Ready Flag: STATUS(1)
          ext_status_sig <= ext_status_sig and x"FFFFFFFA"; 		-- clear ReadReady and WriteReady

				when SRE_B1 =>		-- RESET the RX FIFO
				  addr <= "000";
				  TxRx_sel <= '0';
					readWrite <= '0';
					chipSelect <= '1';
					write_error <= '0';
                -- Clear Read Ready Flag: STATUS(1)
					ext_status_sig <= ext_status_sig and x"FFFFFFFA"; 		-- clear ReadReady and WriteReady
					
				when SRE_B2 => 
				  addr <= "000"; 
				  TxRx_sel <= '0';
					readWrite <= '0';
					chipSelect <= '0';
					write_error <= '0';
                -- Clear Read Ready Flag: STATUS(1)
					ext_status_sig <= ext_status_sig and x"FFFFFFFA"; 		-- clear ReadReady and WriteReady
					
				when SRE_B3 =>		-- potentially add a timeout to reduce risk of infinite loop ???
					addr <= "000";
					TxRx_sel <= '0';    -- 0 is rx
					readWrite <= '0';
					chipSelect <= '1';
					write_error <= '0';
                -- Set Read Ready Flag: STATUS(1)
          ext_status_sig <= ext_status_sig and x"FFFFFFFA"; 		-- clear ReadReady and WriteReady
               	 
				when SRE_B4 =>
					addr <= "000"; 
				  TxRx_sel <= '0';
					readWrite <= '0';
					chipSelect <= '0';
					write_error <= '0';
                -- Clear Read Ready Flag: STATUS(1)
					ext_status_sig <= ext_status_sig and x"FFFFFFFA"; 		-- clear ReadReady and WriteReady
               
				when SRE_B5 =>		-- RESET the RX FIFO and FIFO byte count
				  TxRx_sel <= '0';
				  addr <= "000";									
					readWrite <= '0';
					chipSelect <= '1';
					write_error <= '0';
                -- Clear Read Ready Flag: STATUS(1)
          ext_status_sig <= ext_status_sig and x"FFFFFFFA"; 		-- clear ReadReady and WriteReady

				when SRE_B6 =>		-- RESET the RX FIFO
				  addr <= "000";
				  TxRx_sel <= '0';
					readWrite <= '0';
					chipSelect <= '0';
					write_error <= '0';
                -- Clear Read Ready Flag: STATUS(1)
					ext_status_sig <= ext_status_sig and x"FFFFFFFA"; 		-- clear ReadReady and WriteReady
					
				when SRE_B7 => 
				  addr <= "000"; 
				  TxRx_sel <= '0';
					readWrite <= '0';
					chipSelect <= '1';
                -- Clear Read Ready Flag: STATUS(1)
					ext_status_sig <= ext_status_sig and x"FFFFFFFA"; 		-- clear ReadReady and WriteReady
					
				when SRE_B8 =>		-- potentially add a timeout to reduce risk of infinite loop ???
					addr <= "000";
					TxRx_sel <= '0';    -- 0 is rx
					readWrite <= '0';
					chipSelect <= '0';
					write_error <= '0';
                -- Set Read Ready Flag: STATUS(1)
          ext_status_sig <= ext_status_sig and x"FFFFFFFA"; 		-- clear ReadReady and WriteReady
			 
			 when SRE_B9 => 
				  addr <= "000"; 
				  TxRx_sel <= '0';
					readWrite <= '0';
					chipSelect <= '1';
                -- Clear Read Ready Flag: STATUS(1)
					ext_status_sig <= ext_status_sig and x"FFFFFFFA"; 		-- clear ReadReady and WriteReady
					
				when SRE_B10 =>		-- potentially add a timeout to reduce risk of infinite loop ???
					addr <= "000";
					TxRx_sel <= '0';    -- 0 is rx
					readWrite <= '0';
					chipSelect <= '0';
					write_error <= '0';
                -- Set Read Ready Flag: STATUS(1)
          ext_status_sig <= ext_status_sig and x"FFFFFFFA"; 		-- clear ReadReady and WriteReady
			 
				when SRE_B11 =>
					addr <= "000"; 
				  TxRx_sel <= '0';
					readWrite <= '0';
					chipSelect <= '1';
					servo_error_temp <= x"000000" & dataOut;
					write_error <= '0';
                -- Clear Read Ready Flag: STATUS(1)
					ext_status_sig <= ext_status_sig and x"FFFFFFFA"; 		-- clear ReadReady and WriteReady
					
					
			  when SRE_ERR =>
					addr <= "000";
					TxRx_sel <= '0';    -- 0 is rx
					readWrite <= '0';
					chipSelect <= '0';
					write_error <= '1';
                -- Set Read Ready Flag: STATUS(1)
          ext_status_sig <= ext_status_sig and x"FFFFFFFA"; 		-- clear ReadReady and WriteReady
			 
			 when SRE_ERR2 =>
					addr <= "000";
					TxRx_sel <= '0';    -- 0 is rx
					readWrite <= '0';
					chipSelect <= '0';
					write_error <= '0';
                -- Set Read Ready Flag: STATUS(1)
          ext_status_sig <= ext_status_sig and x"FFFFFFFA"; 		-- clear ReadReady and WriteReady
               	 
					
				when SRE_B12 =>
					TxRx_sel <= '0';    -- 0 is rx
					chipSelect <= '1';
					write_error <= '0';
                -- Clear Read Ready Flag: STATUS(1)
          ext_status_sig <= ext_status_sig and x"00000004"; 		-- Set ReadReady 
          
        when others =>
               -- Report State Machine error.... or dont
                
        end case;
    end process;
    
    NEXT_STATE_LOGIC: process (cur_state, read_write, Tdma, Rdma, clk)
    begin
        case cur_state is
            when S0 =>
                -- check if UART already intialized
                if read_write = '1' and reset = '1' then
						next_state <= SWR0;
					 else
						next_state <= S0;
					 end if;
					 
            --------------------------------------------------
            -- Setup UART block
            --------------------------------------------------
            when SIU0 =>
                next_state <= SIU_LCR1;
            
				when SIU_LCR1 =>
					next_state <= SIU_BS1;
			
            when SIU_BS1 =>
                next_state <= SIU_BS2;
                
            when SIU_BS2 =>
                next_state <= SIU_BS3;
			
				when SIU_BS3 =>
                next_state <= SIU_BS4;
				
				when SIU_BS4 =>
                next_state <= SIU_LCR2 ;
				
				when SIU_LCR2 =>
					next_state <= SIU_LCR3;
			
				when SIU_LCR3 =>
					next_state <= SIU_IRQ;
			
				when SIU_IRQ=>
					next_state <= SIU_IRQ1;
			
				when SIU_IRQ1=>
					next_state <= SIU_FIFO;
            
				when SIU_FIFO =>
					next_state <= SIU_FIFO1;
				
				when SIU_FIFO1 =>
					next_state <= SIU_FIFO2;
				
				when SIU_FIFO2 =>
					next_state <= S0;
				
            --------------------------------------------------
            -- Write Turn Rate States
            --------------------------------------------------
            when SWR0 =>
                -- check if Uart is ready
					 next_state <= SWR_CALC_CHECKSUM;
					 
            when SWR_CALC_CHECKSUM =>
					next_state <= SWR_B1;
					
            when SWR_B1 =>
					next_state <= SWR_B2;
					
            when SWR_B2 =>
					next_state <= SWR_B3;
					
            when SWR_B3 =>
					next_state <= SWR_B4;
					
            when SWR_B4 =>
					next_state <= SWR_B5;
					
            when SWR_B5 =>
					next_state <= SWR_B6;
					
            when SWR_B6 =>
					next_state <= SWR_B7;
					
            when SWR_B7 =>
					next_state <= SWR_B8;
					
            when SWR_B8 =>
					next_state <= SWR_B9;
					
            when SWR_B9 =>
					next_state <= SWR_B10;
					
					
				when SWR_B10 =>
					if Tdma = '1' then
					  next_state <= SWR_B10;
					else
					  next_state <= SRE0;
					end if;
					
					
				----------------------------------------------------------------------
				-- Read Servo Returned Error
				----------------------------------------------------------------------
        
        when SRE0 =>
				    if Rdma = '0' then
							next_state <= SRE_B1;	
						else
							next_state <= SRE0;
						end if;
					
				when SRE_B1 =>
					if( Rdma = '1') then
						next_state <= SRE_B2;
				   else
						next_state <= SRE_B1;
					end if;
					
				when SRE_B2 =>
				  if Rdma = '0' then
							next_state <= SRE_B3;	
						else
							next_state <= SRE_B2;
						end if;
				      
				when SRE_B3 =>
				  if( Rdma = '1') then
						next_state <= SRE_B4;
				   else
						next_state <= SRE_B3;
					end if;
					
				when SRE_B4 =>					-- RxRdy signal active until FIFO is empty in DMA Mode 0
				  if Rdma = '0' then
							next_state <= SRE_B5;	
						else
							next_state <= SRE_B4;
						end if;
					
				when SRE_B5 =>
				  if( Rdma = '1') then
						next_state <= SRE_B6;
				   else
						next_state <= SRE_B5;
					end if;
					
				when SRE_B6 =>
				  if Rdma = '0' then
							next_state <= SRE_B7;	
						else
							next_state <= SRE_B6;
						end if;
				      
				when SRE_B7 =>
				  if( Rdma = '1') then
						next_state <= SRE_B8;
				   else
						next_state <= SRE_B7;
					end if;
					
				when SRE_B8 =>					-- RxRdy signal active until FIFO is empty in DMA Mode 0
					if Rdma = '0' then
							next_state <= SRE_B9;	
						else
							next_state <= SRE_B8;
						end if;
					
				when SRE_B9 =>
				  if( Rdma = '1') then
						next_state <= SRE_B10;
				   else
						next_state <= SRE_B9;
					end if;
					
				
					
				when SRE_B10 =>					-- RxRdy signal active until FIFO is empty in DMA Mode 0
					if Rdma = '0' then
							next_state <= SRE_B11;	
						else
							next_state <= SRE_B10;
						end if;
					
					
				when SRE_B11 =>
						next_state <= SRE_ERR;
						
				when SRE_ERR =>
						next_state <= SRE_ERR2;
						
				when SRE_ERR2 =>
					if( Rdma = '1') then
						next_state <= SRE_B12;
				   else
						next_state <= SRE_ERR2;
					end if;
					
				when SRE_B12 =>
				  next_state <= S0;
					
            when others =>
                -- Report State Machine error.... or dont
        end case;
    end process;

    STATE_MEMORY: process(clk, reset)
    begin
      if(reset = '0') then
        cur_state <= SIU0;
      elsif(rising_edge(clk) and reset = '1') then
        cur_state <= next_state;
      end if;
    end process;

	STORE_ERROR: process(clk, reset)
	begin
		if(reset = '0') then
        Servo_error_latched <= x"FFFFFFFF";
      elsif(rising_edge(clk) and reset = '1') then
        if(write_error = '1') then
				Servo_error_latched <= Servo_error_temp;
		  end if;
      end if;
	end process;
	
	servo_error <= Servo_error_latched;
end packet_former_arch;