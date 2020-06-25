------------------------------------------------------------------------------
-- Imports
------------------------------------------------------------------------------

library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.STD_LOGIC_ARITH.ALL;
use IEEE.STD_LOGIC_UNSIGNED.ALL;

library UNISIM;
use UNISIM.VComponents.all;

------------------------------------------------------------------------------
-- Entity
------------------------------------------------------------------------------
entity MySystem is
port
(
--#ports

-- b5x300
CLK				: in std_logic;
LED				: out std_logic;
TESTn			: in std_logic;

-- gba_cart_left
GBACART_AH		: in std_logic_vector(7 downto 0);
GBACART_CS2n	: in std_logic;
GBACART_IRQ		: out std_logic;

-- ds_slot
DSSLOT_GND1		: in std_logic;
DSSLOT_CLK		: in std_logic;
DSSLOT_NC		: in std_logic;
DSSLOT_ROMCS	: in std_logic;
DSSLOT_RESET	: in std_logic;
DSSLOT_EEPCS	: in std_logic;
DSSLOT_IRQ		: out std_logic;
DSSLOT_VCC		: in std_logic;
DSSLOT_IO		: inout std_logic_vector(7 downto 0);
DSSLOT_GND2		: in std_logic;

-- gba_cart_right
GBACART_PHI		: in std_logic;
GBACART_WRn		: in std_logic;
GBACART_RDn		: in std_logic;
GBACART_CSn		: in std_logic;
GBACART_AD		: inout std_logic_vector(15 downto 0);

-- lpcflash
LPC_CLK				: out std_logic;
LPC_FRAME			: out std_logic;
LPC_RST				: out std_logic;
LPC_LDA				: inout std_logic_vector(3 downto 0);
LPC_GND				: out std_logic_vector(6 downto 0);

RTC_CS				: in std_logic;
RTC_CLK				: in std_logic;
RTC_DAT				: in std_logic;

-- ds_cart
DSCART_GND		: in std_logic;
DSCART_CLK		: out std_logic;
DSCART_NC		: in std_logic;
DSCART_ROMCS	: out std_logic;
DSCART_RESET	: out std_logic;
DSCART_EEPCS	: out std_logic;
DSCART_IRQ		: in std_logic;
DSCART_VCC		: in std_logic;
DSCART_IO		: inout std_logic_vector(7 downto 0);

-- b5leds
LEDS			: out std_logic_vector(15 downto 0);

-- b5pport
PP_DATA				: in std_logic_vector(7 downto 0);

PP_BUSY				: out std_logic;	-- 0x80^
PP_ACK				: out std_logic;	-- 0x40
PP_PAPER_OUT		: out std_logic;	-- 0x20
PP_SELECT			: out std_logic;	-- 0x10
PP_ERROR			: out std_logic;	-- 0x08

PP_SELECT_PRINTER	: in std_logic;		-- 0x08
PP_INIT_PRINTER		: in std_logic;		-- 0x04
PP_AUTO_LINEFEED	: in std_logic;		-- 0x02
PP_STROBE			: in std_logic;		-- 0x01
--#ports;
		DUMMY : in std_logic
);
end entity;

------------------------------------------------------------------------------
-- Architecture
------------------------------------------------------------------------------
architecture rtl of MySystem is


	signal dsslot_clk_buf : std_logic;
	signal dsslot_romcs_buf : std_logic;
	signal dsslot_vcc_buf : std_logic;
	
	
	signal counter : std_logic_vector(15 downto 0) := (others => '0');
	
	signal transfer_cnt : natural range 0 to 16383 := 0;
	signal cmd_cnt : natural range 0 to 7 := 0;
	signal data_cnt : natural range 0 to 8191 := 0;
	signal prev_data_cnt : natural range 0 to 8191 := 0;
	
	signal is_command : boolean := true;

	type dos_type is array(9 downto 0) of std_logic_vector(7 downto 0);

	signal pp_strobe_buf : std_logic;	--_vector(2 downto 0);
	signal pp_data_out : std_logic_vector(7 downto 0);


	signal patch_en : boolean := false;
	signal capture_en : boolean := false;

------------------------------------------------------------------------------
-- blockrams
------------------------------------------------------------------------------

	signal pp_bram_clk : std_logic;
	signal patch_bram_clk : std_logic;
	signal size_bram_clk : std_logic;
	signal cmd_bram_clk : std_logic;
	signal gba_bram_clk : std_logic;

	signal pp_bram_addr : std_logic_vector(4+8 downto 0);
	signal patch_bram_addr : std_logic_vector(4+8 downto 0);
	signal size_bram_addr : std_logic_vector(4+8 downto 0);
	signal cmd_bram_addr : std_logic_vector(4+8 downto 0);
	signal gba_bram_addr_hi : std_logic_vector(7 downto 0);
	signal gba_bram_addr_lo : std_logic_vector(15 downto 0);
	signal gba_bram_addr : std_logic_vector(23 downto 0);

	signal pp_bram_num : natural range 0 to 9;
	signal patch_bram_num : natural range 0 to 9;
	signal size_bram_num : natural range 0 to 9;
	signal cmd_bram_num : natural range 0 to 9;
	signal gba_bram_num : natural range 0 to 9;

	signal pp_bram_ens : std_logic_vector(9 downto 0);
	signal patch_bram_ens : std_logic_vector(9 downto 0);
	signal size_bram_ens : std_logic_vector(9 downto 0);
	signal cmd_bram_ens : std_logic_vector(9 downto 0);
	--signal gba_bram_ens : std_logic_vector(9 downto 0);

	signal pp_bram_we : std_logic;
	signal patch_bram_we : std_logic;
	signal size_bram_we : std_logic;
	signal cmd_bram_we : std_logic;
	signal gba_bram_we : std_logic;

	signal pp_bram_di : std_logic_vector(7 downto 0);
	signal patch_bram_di : std_logic_vector(7 downto 0);
	signal size_bram_di : std_logic_vector(7 downto 0);
	signal cmd_bram_di : std_logic_vector(7 downto 0);
	signal gba_bram_di : std_logic_vector(15 downto 0);

	signal pp_bram_do : std_logic_vector(7 downto 0);
	signal patch_bram_do : std_logic_vector(7 downto 0);
	signal size_bram_do : std_logic_vector(7 downto 0);
	signal cmd_bram_do : std_logic_vector(7 downto 0);
	signal gba_bram_do : std_logic_vector(15 downto 0);

	signal pp_bram_dos : dos_type;
	signal patch_bram_dos : dos_type;
	signal size_bram_dos : dos_type;
	signal cmd_bram_dos : dos_type;

------------------------------------------------------------------------------
-- GBA slot
------------------------------------------------------------------------------

	signal GBACART_RDn_buf : std_logic;
	signal GBACART_WRn_buf : std_logic;
	signal GBACART_CSn_buf : std_logic;

------------------------------------------------------------------------------
-- RTC
------------------------------------------------------------------------------

	signal rtc_clk_buf : std_logic;
	signal rtc_count : std_logic_vector(15 downto 0) := X"AAAA";

------------------------------------------------------------------------------
-- Implementation
------------------------------------------------------------------------------
begin

------------------------------------------------------------------------------
-- Misc
------------------------------------------------------------------------------

--LEDS <= PROBES(15 downto 0);	--PP_DATA & PP_DATA;


------------------------------------------------------------------------------
-- blockrams
------------------------------------------------------------------------------


	patch_bram: for i in 0 to 0 generate
	begin

		bram_patch : RAMB4_S8_S8
		port map
		(
			RSTA => '0',
			CLKA => pp_bram_clk,
			ENA => pp_bram_ens(i),
			WEA => pp_bram_we,
			ADDRA => pp_bram_addr(8 downto 0),
			DOA => pp_bram_dos(i),
			DIA => pp_bram_di,

			RSTB => '0',
			CLKB => patch_bram_clk,
			ENB => patch_bram_ens(i),
			WEB => patch_bram_we,
			ADDRB => patch_bram_addr(8 downto 0),
			DOB => patch_bram_dos(i),
			DIB => patch_bram_di
		);

		pp_bram_ens(i) <= '1' when (i = pp_bram_num) else '0';
		patch_bram_ens(i) <= '1' when (i = patch_bram_num) else '0';
	
	end generate;


	size_bram: for i in 1 to 1 generate
	begin

		bram_size : RAMB4_S8_S8
		port map
		(
			RSTA => '0',
			CLKA => pp_bram_clk,
			ENA => pp_bram_ens(i),
			WEA => pp_bram_we,
			ADDRA => pp_bram_addr(8 downto 0),
			DOA => pp_bram_dos(i),
			DIA => pp_bram_di,

			RSTB => '0',
			CLKB => size_bram_clk,
			ENB => size_bram_ens(i),
			WEB => size_bram_we,
			ADDRB => size_bram_addr(8 downto 0),
			DOB => size_bram_dos(i),
			DIB => size_bram_di
		);

		pp_bram_ens(i) <= '1' when (i = pp_bram_num) else '0';
		size_bram_ens(i) <= '1' when (i = size_bram_num) else '0';
	
	end generate;


	cmd_bram: for i in 2 to 8 generate
	begin

		bram_cmd : RAMB4_S8_S8
		port map
		(
			RSTA => '0',
			CLKA => pp_bram_clk,
			ENA => pp_bram_ens(i),
			WEA => pp_bram_we,
			ADDRA => pp_bram_addr(8 downto 0),
			DOA => pp_bram_dos(i),
			DIA => pp_bram_di,

			RSTB => '0',
			CLKB => cmd_bram_clk,
			ENB => cmd_bram_ens(i),
			WEB => cmd_bram_we,
			ADDRB => cmd_bram_addr(8 downto 0),
			DOB => cmd_bram_dos(i),
			DIB => cmd_bram_di
		);

		pp_bram_ens(i) <= '1' when (i = pp_bram_num) else '0';
		cmd_bram_ens(i) <= '1' when (i = cmd_bram_num) else '0';
	
	end generate;


	pp_bram_clk <= pp_strobe_buf;
	patch_bram_clk <= not dsslot_clk_buf;
	cmd_bram_clk <= dsslot_clk_buf;
	size_bram_clk <= dsslot_clk_buf;

	LED <= '1' when capture_en else '0';

	capture_en <= (size_bram_addr < "0010" & "000000000") and (cmd_bram_addr < "1001" & "000000000");
	patch_en <= (transfer_cnt = 1);


	pp_bram_we <= PP_SELECT_PRINTER;
	patch_bram_we <= '0';
	cmd_bram_we <= '1' when is_command and dsslot_romcs_buf='0' and capture_en else '0';
	size_bram_we <= '1' when is_command and dsslot_romcs_buf='0' and capture_en and (cmd_cnt < 2) else '0';


	pp_bram_di <= PP_DATA;
	patch_bram_di <= (others => '0');
	cmd_bram_di <= DSSLOT_IO;
	size_bram_di <= 
		conv_std_logic_vector(prev_data_cnt,8) when size_bram_addr(0)='0' else
		conv_std_logic_vector(prev_data_cnt/256,8)
	;


	pp_bram_num <= conv_integer(pp_bram_addr(12 downto 9));
	patch_bram_num <= conv_integer(patch_bram_addr(12 downto 9));
	cmd_bram_num <= conv_integer(cmd_bram_addr(12 downto 9));
	size_bram_num <= conv_integer(size_bram_addr(12 downto 9));


	pp_data_out <= pp_bram_do;

	pp_bram_do <= pp_bram_dos(pp_bram_num);
	patch_bram_do <= patch_bram_dos(patch_bram_num);
	--cmd_bram_do <= cmd_bram_dos(cmd_bram_num);
	--size_bram_do <= size_bram_dos(size_bram_num);


------------------------------------------------------------------------------
-- Parallel port
------------------------------------------------------------------------------


	PP_BUSY <= pp_data_out(7) when PP_AUTO_LINEFEED='1' else pp_data_out(3);
	PP_ACK <= pp_data_out(6) when PP_AUTO_LINEFEED='1' else pp_data_out(2);
	PP_PAPER_OUT <= pp_data_out(5) when PP_AUTO_LINEFEED='1' else pp_data_out(1);
	PP_SELECT <= pp_data_out(4) when PP_AUTO_LINEFEED='1' else pp_data_out(0);


--PP_SELECT_PRINTER	: in std_logic;		-- 0x08				write enable
--PP_INIT_PRINTER		: in std_logic;		-- 0x04			set address (PP_AUTO_LINEFEED selects low/high byte)
--PP_AUTO_LINEFEED	: in std_logic;		-- 0x02				nibble
--PP_STROBE			: in std_logic;		-- 0x01				clk

	process (CLK)
	begin
		if (rising_edge(CLK)) then
			pp_strobe_buf <= PP_STROBE;
		end if;
	end process;


	process (pp_bram_clk)
	begin
		if (PP_INIT_PRINTER='1') then
			--pp_bram_addr <= "0000" & "000000000";
			if (PP_AUTO_LINEFEED='0') then
				pp_bram_addr(7 downto 0) <= PP_DATA;
			else
				pp_bram_addr(4+8 downto 8) <= PP_DATA(4 downto 0);
			end if;
		elsif (falling_edge(pp_bram_clk)) then
			pp_bram_addr <= pp_bram_addr + 1;
		end if;
	end process;




	
	--LED <= cmd_bram_we;
	--LEDS(4+8 downto 0) <= cmd_bram_addr;	--pp_bram_addr;
	--LEDS <= conv_std_logic_vector(transfer_cnt, 16);


------------------------------------------------------------------------------
-- DS slot
------------------------------------------------------------------------------

	dsslot_vcc_ibuf : IBUF port map ( I => DSSLOT_VCC, O => dsslot_vcc_buf );

	dsslot_clk_ibuf : IBUF port map ( I => DSSLOT_CLK, O => dsslot_clk_buf );
	dsslot_clk_obuf : OBUF port map ( I => dsslot_clk_buf, O => DSCART_CLK );

	dsslot_romcs_ibuf : IBUF port map ( I => DSSLOT_ROMCS, O => dsslot_romcs_buf );
	dsslot_romcs_obuf : OBUF port map ( I => dsslot_romcs_buf, O => DSCART_ROMCS );


	DSCART_RESET <= DSSLOT_RESET;
	DSCART_EEPCS <= DSSLOT_EEPCS;
	DSSLOT_IRQ <= DSCART_IRQ;



	process (CLK)
	begin

		DSSLOT_IO <= (others => 'Z');
		DSCART_IO <= (others => 'Z');

		if (DSSLOT_RESET='1') then

			if (DSSLOT_EEPCS='0') then
				DSCART_IO(7) <= DSSLOT_IO(7);
				DSSLOT_IO(6) <= DSCART_IO(6);
			end if;

			if (dsslot_romcs_buf='0') then

				if (not is_command) then
					if (patch_en) then
						DSSLOT_IO <= patch_bram_do;	-- own data
					else
						DSSLOT_IO <= DSCART_IO;		-- data
					end if;
				end if;

				if (is_command) then
					--if (cmd_cnt=0 and TESTn='0') then
						--DSCART_IO <= DSSLOT_IO xor X"0F";
					--else
						DSCART_IO <= DSSLOT_IO;		-- command
					--end if;
				end if;

			end if;

		end if;

	end process;


	process (dsslot_romcs_buf)
	begin
		if (DSSLOT_RESET='0') then
			transfer_cnt <= 0;
		elsif rising_edge(dsslot_romcs_buf) then
			transfer_cnt <= transfer_cnt + 1;
		end if;
	end process;



	process (patch_bram_clk)
	begin
	
		if (rising_edge(patch_bram_clk)) then

			if (cmd_cnt = 7) then
				patch_bram_addr <= "0000" & "000000000";
			end if;

			if (not is_command) then
				patch_bram_addr <= patch_bram_addr + 1;
			end if;
		
		end if;

	end process;



	process (size_bram_clk, dsslot_vcc_buf)
	begin
		if (dsslot_vcc_buf='0') then

			size_bram_addr <= "0001" & "000000000";

		elsif (rising_edge(size_bram_clk)) then

			if (size_bram_we='1') then
				size_bram_addr <= size_bram_addr + 1;
			end if;

		end if;
	end process;



	process (cmd_bram_clk, dsslot_vcc_buf)
	begin
		if (dsslot_vcc_buf='0') then

			cmd_bram_addr <= "0010" & "000000000";

		elsif (rising_edge(cmd_bram_clk)) then

			if (cmd_bram_we='1') then
				cmd_bram_addr <= cmd_bram_addr + 1;
			end if;

		end if;
	end process;




	process (dsslot_clk_buf)
	begin

		if (dsslot_romcs_buf='1') then

			cmd_cnt <= 0;
			prev_data_cnt <= data_cnt;
			is_command <= true;

		elsif (rising_edge(dsslot_clk_buf)) then

			if (cmd_cnt = 7) then
				is_command <= false;
				data_cnt <= 0;
			end if;

			if (is_command) then
				cmd_cnt <= cmd_cnt + 1;
			else
				data_cnt <= data_cnt + 1;
			end if;

		end if;

	end process;


------------------------------------------------------------------------------
-- GBA slot
------------------------------------------------------------------------------

	gba_bram: for i in 9 to 9 generate
	begin

		bram_gba : RAMB4_S8_S16
		generic map
		(
	        INIT_00 => X"19BE52A3217F81C0988B2411AD09E4840A82843D21A29A6951AEFF24EA00002E",
	        INIT_01 => X"C08A5694C1094BCE94DFF485BFCEE38233E8C758EC3127F84A4A461020CE0993",
	        INIT_02 => X"008438BF56AE040361C71D23769803FC27A39758619ACAA3734D849FFCA77213",
	        INIT_03 => X"FF34A2F9E2384E0103BE63A92580D66085C0FB97F130956F03FE52FFFD0EA740",
	        INIT_04 => X"07F8D42172AC0A388BE425D6AF3CF087637CC065943A1188CB90007844033EBB",
	        INIT_05 => X"000088000000000000000000009631304A5A46414E41564441204F52455A2D46",
	        INIT_06 => X"E28F0018E59F1154E59FD018E129F000E3A0001FE59FD028E129F000E3A00012",
	        INIT_07 => X"E3A0330103007FA003007E00EAFFFFF2E12FFF11E1A0E00FE59F114CE5810000",
	        INIT_08 => X"E3A02000E0021822E92D400BE14F0000E1A01821E1A01802E5932000E2833C02",
	        INIT_09 => X"1A000020E2110040E28220041A000039E2110004E28220041A000026E2110080",
	        INIT_0A => X"E2110008E28220041A00001AE2110002E28220041A00001DE2110001E2822004",
	        INIT_0B => X"E28220041A000011E2110020E28220041A000014E2110010E28220041A000017",
	        INIT_0C => X"1A000008E2110B01E28220041A00000BE2110C02E28220041A00000EE2110C01",
	        INIT_0D => X"E2110A02E28220041A000002E2110A01E28220041A000005E2110B02E2822004",
	        INIT_0E => X"E129F003E383301FE3C330DFE10F3000E1C310B0E59F106CE1C300B21AFFFFFE",
	        INIT_0F => X"E10F3000E8BD4000E12FFF10E28FE000E92D4000E5910000E0811002E59F1058"
		)
		port map
		(
			RSTA => '0',
			CLKA => pp_bram_clk,
			ENA => pp_bram_ens(i),
			WEA => pp_bram_we,
			ADDRA => pp_bram_addr(8 downto 0),
			DOA => pp_bram_dos(i),
			DIA => pp_bram_di,
	
			RSTB => '0',
			CLKB => gba_bram_clk,
			ENB => '1',	--gba_bram_ens(),
			WEB => gba_bram_we,
			ADDRB => gba_bram_addr_lo(7 downto 0),
			DOB => gba_bram_do,
			DIB => gba_bram_di
		);

		pp_bram_ens(i) <= '1' when (i = pp_bram_num) else '0';
		--gba_bram_ens(i) <= '1' when (i = gba_bram_num) else '0';

	end generate;

	--GBACART_RDn_ibuf : IBUF port map ( I => GBACART_RDn, O => GBACART_RDn_buf );
	--GBACART_WRn_ibuf : IBUF port map ( I => GBACART_WRn, O => GBACART_WRn_buf );
	--GBACART_CSn_ibuf : IBUF port map ( I => GBACART_CSn, O => GBACART_CSn_buf );

	--LED <= gba_bram_addr(0);
	--LEDS <= gba_bram_addr_hi & "00000000";
	LEDS <= gba_bram_addr(15 downto 0);

	gba_bram_clk <= not GBACART_RDn_buf or not GBACART_WRn_buf;
	--gba_bram_addr_hi <= GBACART_A;
	gba_bram_addr <= gba_bram_addr_hi & gba_bram_addr_lo;
	GBACART_IRQ <= '0';
--	gba_bram_we <= '1' when GBACART_WRn_buf='0' and GBACART_CSn_buf='0' and gba_bram_addr=X"AAAAAA" else '0';
	gba_bram_di <= GBACART_AD;
--	GBACART_AD <=
--		gba_bram_do when GBACART_RDn_buf='0' and GBACART_CSn_buf='0' and gba_bram_addr<X"000100"
--		else (others => 'Z');


	process (GBACART_WRn_buf, GBACART_RDn_buf, GBACART_CSn_buf, gba_bram_addr, gba_bram_do)
	begin

		GBACART_AD <= (others => 'Z');
		if (GBACART_RDn_buf='0' and GBACART_CSn_buf='0') then
		 	if (gba_bram_addr < X"000100") then
				GBACART_AD <= gba_bram_do;
			else
				case gba_bram_addr(2 downto 0) is
					when "000" => GBACART_AD <= rtc_count;
					when others => GBACART_AD <= (others => '1');
				end case;
			end if;
		end if;

		gba_bram_we <= '0';
		if (GBACART_WRn_buf='0' and GBACART_CSn_buf='0' and gba_bram_addr<X"000100") then
			gba_bram_we <= '1';
		end if;

	end process;
	
-- gba_bram_addr is 16 bits!


	process (CLK)
	begin
		if (rising_edge(CLK)) then
			GBACART_RDn_buf <= GBACART_RDn;
			GBACART_WRn_buf <= GBACART_WRn;
			GBACART_CSn_buf <= GBACART_CSn;
		end if;
	end process;

	
	process (GBACART_CSn_buf, GBACART_RDn_buf)
	begin

		if (GBACART_CSn_buf='1') then
			gba_bram_addr_hi <= GBACART_AH;
			gba_bram_addr_lo <= GBACART_AD;
		elsif (rising_edge(GBACART_RDn_buf)) then
			gba_bram_addr_lo <= gba_bram_addr_lo + 1;
		end if;
	
	end process;


------------------------------------------------------------------------------
-- RTC
------------------------------------------------------------------------------

	--rtc_clk_ibuf : IBUF port map ( I => RTC_CLK, O => rtc_clk_buf );

	process (CLK)
	begin

		if (rising_edge(CLK)) then
			rtc_clk_buf <= RTC_CLK;
		end if;
	
	end process;

	process (rtc_clk_buf, TESTn)
	begin

		if (TESTn='0') then
			rtc_count <= (others => '0');
		elsif (rising_edge(rtc_clk_buf)) then
			rtc_count <= rtc_count + 1;
		end if;
	
	end process;

	--LEDS <= rtc_count;


------------------------------------------------------------------------------
-- End
------------------------------------------------------------------------------

end architecture;
