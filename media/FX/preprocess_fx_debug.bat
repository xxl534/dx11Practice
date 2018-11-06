@echo off
For /r . %%x in (*.fx) do (
call fxc /Fc /Od /Zi /T fx_5_0 /Fo "%%xo" "%%x"
echo "%%x" compiled
)
pause>nul