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

	signal PP_STROBE_r : std_logic;

------------------------------------------------------------------------------
-- Implementation
------------------------------------------------------------------------------
begin

------------------------------------------------------------------------------
-- Misc
------------------------------------------------------------------------------

	DSCART_CLK <= PP_STROBE_r;
	DSCART_ROMCS <= PP_SELECT_PRINTER;
	DSCART_RESET <= '0' when PP_AUTO_LINEFEED='0' and PP_INIT_PRINTER='0' else '1';
	DSCART_EEPCS <= '1';
	DSCART_IO <= PP_DATA when PP_INIT_PRINTER='0' else (others => 'Z');

	PP_BUSY <= DSCART_IO(7) when PP_AUTO_LINEFEED='1' else DSCART_IO(3);
	PP_ACK <= DSCART_IO(6) when PP_AUTO_LINEFEED='1' else DSCART_IO(2);
	PP_PAPER_OUT <= DSCART_IO(5) when PP_AUTO_LINEFEED='1' else DSCART_IO(1);
	PP_SELECT <= DSCART_IO(4) when PP_AUTO_LINEFEED='1' else DSCART_IO(0);
	PP_ERROR <= '0';

	LED <= PP_INIT_PRINTER;		-- output mode

	process (CLK)
	begin
		if (rising_edge(CLK)) then
			PP_STROBE_r <= PP_STROBE;
		end if;
	end process;

------------------------------------------------------------------------------
-- End
------------------------------------------------------------------------------

end architecture;
