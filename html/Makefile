
# Makefile to create JWlink.chm/JWlink.chi
# input files:
# - JWlink.htm
# - JWlink.mix

hbook=JWlink
OUTD=Win32
hc_hh = "d:\tools\html_help_workshop\hhc.exe"

.EXTENSIONS: .chm .chi

$(OUTD)\$(hbook).chm $(OUTD)\$(hbook).chi : $(hbook).htm $(hbook).mix
    @if not exist $(OUTD) mkdir $(OUTD)
    awk -f splithtm.awk -v dir=$(OUTD) $(hbook).htm
    awk -f makehhc.awk $(hbook).htm >$(OUTD)/$(hbook).hhc
    awk -f makehhk.awk $(hbook).mix >$(OUTD)/$(hbook).hhk
    awk -f makehhp.awk -v hhcfile=$(hbook).hhc -v hhkfile=$(hbook).hhk $(hbook).htm >$(OUTD)/$(hbook).hhp
    -$(hc_hh) $(OUTD)/$(hbook).hhp

