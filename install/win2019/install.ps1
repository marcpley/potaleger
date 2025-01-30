$desktop = [System.Environment]::GetFolderPath("Desktop")
Copy-Item -Path $PSScriptRoot\potaleger.lnk -Destination $desktop\potaleger.lnk -Force
