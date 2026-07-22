## Example Summary

This example demonstrates the use of Type B EEPROM emulation to store user data.

Type B EEPROM Emulation maximizes the lifetime of Flash memory when writing individual words of non-volatile data.
Please check the Type A implementation when writing blocks of data.

## Peripherals & Pin Assignments
- FlashCtl

## BoosterPacks, Board Resources & Jumper Settings
N/A


### Device Migration Recommendations
This project was developed for a superset device included in the LP_MSPM0G3507 LaunchPad. Please
visit the [CCS User's Guide](https://software-dl.ti.com/msp430/esd/MSPM0-SDK/latest/docs/english/tools/ccs_ide_guide/doc_guide/doc_guide-srcs/ccs_ide_guide.html#non-sysconfig-compatible-project-migration)
for information about migrating to other MSPM0 devices.

## Example Usage

Compile, load and run the example.
After initialization, part of the Flash area will be used as emulated EEPROM.
In the application, the data is saved to the emulated EEPROM 255 times, with the
255th iteration erasing the block for FLASH which had been written to.
The application will stop at breakpoints so the user can verify the values.

## Library Notes

After running DL_FlashCTL_eraseMemoryFromRAM to erase sectors in a group, all bytes are reset to 0xFF.
To protect from a power outage while the erase operation runs, after a successful erase, the header
of the group is set to the confirmed erase state (0xFF00FFFF 0xFFFFFFFF).

EEPROM_EMULATION_INIT_FORCE_ERASE_NOACTIVE (default=0). At bootup, the init function
erases all non-active groups. The default behavior of this flag is to skip erasing
groups which have headers in the confirmed erased (0xFF00FFFF 0xFFFFFFFF) state to prevent
wasting unnecessary flash write cycles. By setting this flag to 1, groups in the confirmed erased
state will be re-erased.

Regardless of the value of this flag, since this library treats a group with header
(0xFFFFFFFF 0xFFFFFFFF) as unknown rather than a confirmed erase, it will always run another
erase operation. This is because (0xFFFFFFFF 0xFFFFFFFF) is the hardware default state after
erase and cannot be distinguished from a sector whose header cell happened to erase first
during a power loss event.
