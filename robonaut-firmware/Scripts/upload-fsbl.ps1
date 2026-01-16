$BuildType = $args[0]
$STM32ProgrammerPath = $args[1]
$ProjectName = $args[2]

$fsblBinPath = "FSBL/build/$BuildType/robonaut-firmware_FSBL.bin"
$fsblTrustedBinPath = "FSBL/build/$BuildType/robonaut-firmware_FSBL-trusted.bin"
"y" | & "$STM32ProgrammerPath/STM32_SigningTool_CLI.exe" -bin $fsblBinPath -nk -of 0x80000000 -t fsbl -o $fsblTrustedBinPath -hv 2.3 -dump $fsblTrustedBinPath -align

& "$STM32ProgrammerPath/STM32_Programmer_CLI" -c port=SWD mode=Normal ap=1 -w FSBL/build/$BuildType/${ProjectName}_FSBL-trusted.bin 0x70000000 `
        -v -el MemLoader/RobonAUT_FSBL_XIP_ExtMemLoader.stldr -hardRst