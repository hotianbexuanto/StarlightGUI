@echo off
chcp 65001
title Starlight GUI 服务修复
echo 删除旧服务中...
sc stop "StarlightGUI Kernel Driver"
sc delete "StarlightGUI Kernel Driver"
sc stop "AstralX"
sc delete "AstralX"
echo.
echo 操作已完成.
echo 若提示拒绝访问(Access is denied, 0x5)，请使用管理员权限启动本脚本.
echo 你可以关闭本窗口了.
pause