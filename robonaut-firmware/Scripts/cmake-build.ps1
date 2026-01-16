$BuildType = $args[0]
$STM32ProgrammerPath = $args[1]

Write-Host "Selected build Type: $BuildType"

cmake --preset $BuildType
cmake --build --preset $BuildType
