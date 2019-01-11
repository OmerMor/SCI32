del *.sl
del *.scr
del *.hep

del selector.
copy selector.new selector

echo.                       > remake.err
echo Remaking system       >> remake.err
echo.                      >> remake.err
sc -dl -O @remake.lst 		>> remake.err

