
# Makefile to create JWlink.chm/JWlink.chi
# input files:
# - JWlink.htm
# - JWlink.mix

hbook=JWlink
OUTD=Win32
hc_hh = "\htmlhelp\hhc.exe"

.EXTENSIONS: .chm .chi

$(OUTD)\$(hbook).chm $(OUTD)\$(hbook).chi : $(OUTD)\$(hbook).hhc $(OUTD)\$(hbook).hhk $(OUTD)\$(hbook).hhp
	@cd $(OUTD)
    -$(hc_hh) $(hbook).hhp
	@cd ..

$(OUTD)\$(hbook).hhc $(OUTD)\$(hbook).hhk $(OUTD)\$(hbook).hhp : $(hbook).html $(hbook).mix
    @if not exist $(OUTD) mkdir $(OUTD)
    awk -f splithtm.awk -v dir=$(OUTD) $(hbook).html
    awk -f makehhc.awk $(hbook).html >$(OUTD)\$(hbook).hhc
    awk -f makehhk.awk $(hbook).mix >$(OUTD)\$(hbook).hhk
    awk -f makehhp.awk -v hhcfile=$(hbook).hhc -v hhkfile=$(hbook).hhk $(hbook).html >$(OUTD)\$(hbook).hhp
