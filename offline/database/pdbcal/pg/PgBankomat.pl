#-----------------------------------------------------------------------------
#  $Header: /afs/rhic.bnl.gov/phenix/PHENIX_CVS/offline/database/pdbcal/pg/PgBankomat.pl,v 1.9 2008/10/17 15:22:33 irina Exp $
#
#  The pdbcal package
#  Copyright (C) PHENIX collaboration, 1999
#
#  Perl script PdbBankomat.pl
#
#  Purpose: auto-generate banks
#
#  Description:
#
#  Author: Matthias Messer
#-----------------------------------------------------------------------------

if ($#ARGV > -1) {
  #
  # Anybody asked for help?
  #
  if ($ARGV[0] eq "-h" || $ARGV[0] eq "--help" || $ARGV[0] eq "-?") {
    print "\n";
    print "******************** bankomat, the bank-generator *********************** \n";
    print "--------------------------------------------------------------------------\n";
    print "arguments:                                                                \n";
    print " -1- The name of the class to be stored, e.g. PdbADCChan,                 \n";
    print "                                                                          \n";
    print " The script should be called in the make process                          \n";
    print "                                          8/1999 Matthias Messer          \n";
    print "\n";
    exit;
  }
}
if ($#ARGV == 0) {
  $className = $ARGV[0];
} else {
  print"usage PdbBankomat [-h] [--help] [-?] storageClass";
  exit;
}

if ($className =~ /PgPost/)
{
#strip the . since we are called with either .h or .cc
    @sp1 = split(/\./, $className);
    $bankName = $sp1[0];
    $pdbname = $sp1[0];
    $pdbname =~ s/PgPost/Pdb/;
    $pdbname =~ s/Bank//;
}
else
{
    @nameList = split(/Pg/, $className);
    $bankName  = join ("", "PgPost", $nameList[1], "Bank");
    $pdbname = join ("", "Pdb", $nameList[1]);
}

#
# Write  bank ddl-file
#
open (BANKHEADER, ">$bankName.h") || die "cannot open bank header file $bankName.ddl\n";

print BANKHEADER "//-----------------------------------------------------------------------------\n";
print BANKHEADER "//\n";
print BANKHEADER "//  The pdbcal package\n";
print BANKHEADER "//  Copyright (C) PHENIX collaboration, 1999\n";
print BANKHEADER "//\n";
print BANKHEADER "//  Declaration of class $bankName\n";
print BANKHEADER "//\n";
print BANKHEADER "//  Purpose:  bank for user class $className\n";
print BANKHEADER "//\n";
print BANKHEADER "//  Description:\n";
print BANKHEADER "//\n";
print BANKHEADER "//  Author: auto-generated by PgBankomat.pl\n";
print BANKHEADER "//-----------------------------------------------------------------------------\n";
$capBankName = uc $bankName;
$bankIfDef   = join("", "__", $capBankName, "_HH__");
print BANKHEADER "#ifndef $bankIfDef\n";
print BANKHEADER "#define $bankIfDef\n";
print BANKHEADER "\n";
print BANKHEADER "#include \"PgPostCalBank.h\"\n";
print BANKHEADER "#include <pdbcalbase/$pdbname.h>\n";
print BANKHEADER "#include <vector>\n";
print BANKHEADER "#include <iostream>\n";
print BANKHEADER "\n";
print BANKHEADER "class $bankName : public PgPostCalBank {\n";
print BANKHEADER " public:\n";
print BANKHEADER "  $bankName(); \n";
print BANKHEADER "  ~$bankName() override;\n";
print BANKHEADER "\n";
print BANKHEADER "  void print() override;\n";
print BANKHEADER "  void printEntry(size_t i) override { entries[i].print(); }\n";
print BANKHEADER "  void printHeader() const override { ; }\n";
print BANKHEADER "   \n";
print BANKHEADER "  PdbCalChan & getEntry(size_t i) override { return entries[i]; }\n";
print BANKHEADER "  size_t getLength() override { return entries.size(); }\n";
print BANKHEADER "  void setLength(size_t val) override { entries.resize(val); }\n";
print BANKHEADER "  int isValid(const PHTimeStamp &) const override { return 0; }\n";
print BANKHEADER "  virtual bool commit() { std::cout << \"Can commit only a wrapper\" << std::endl;return 0; } \n";
print BANKHEADER "   \n";
print BANKHEADER "  PHObject* CloneMe() const override;\n";
print BANKHEADER " private:\n";
print BANKHEADER "  std::vector<$pdbname> entries;\n\n";
print BANKHEADER "  ClassDefOverride($bankName,1); \n";
print BANKHEADER "};\n";
print BANKHEADER "\n";
print BANKHEADER "#endif /* $bankIfDef */\n";

close BANKHEADER;

#
# Write  bank implementation file
#
open (BANKIMP, ">$bankName.cc") || die "cannot open bank implementation file $bankName.cc\n";

print BANKIMP "//-----------------------------------------------------------------------------\n";
print BANKIMP "//\n";
print BANKIMP "//  The pdbcal package\n";
print BANKIMP "//  Copyright (C) PHENIX collaboration, 1999\n";
print BANKIMP "//\n";
print BANKIMP "//  Implementation of class $bankName\n";
print BANKIMP "//\n";
print BANKIMP "//  Author: auto-generated by PdbBankomat.pl\n";
print BANKIMP "//-----------------------------------------------------------------------------\n";
print BANKIMP "#include \"$bankName.h\"\n";
print BANKIMP "#include <pdbcalbase/PdbClassMap.h>\n";
print BANKIMP "#include <iostream>\n";
print BANKIMP "\n";
print BANKIMP "$bankName\:\:$bankName()\n";
print BANKIMP "{\n";
print BANKIMP "}\n";
print BANKIMP "\n";
print BANKIMP "$bankName\:\:~$bankName()\n";
print BANKIMP "{\n";
print BANKIMP "}\n";
print BANKIMP "\n";
print BANKIMP "PHObject* $bankName\:\:CloneMe() const\n";
print BANKIMP "{\n";
print BANKIMP " return new $bankName(*this);\n";
print BANKIMP "}\n";
print BANKIMP "\n";
print BANKIMP "void $bankName\:\:print()\n";
print BANKIMP "{\n";
print BANKIMP "   std::cout << \"number of entries in  $bankName:  \" << entries.size() << std::endl;\n";
print BANKIMP "}\n";
print BANKIMP "\n";
$regName = join("", $bankName, "Register");
print BANKIMP "class $regName {\n";
print BANKIMP "public:\n";
print BANKIMP "   $regName()\n";
print BANKIMP "      {\n";
print BANKIMP "         PdbClassMap<PdbCalBank> *classMap = PdbClassMap<PdbCalBank>::instance();\n";
print BANKIMP "         $bankName * pointer = new $bankName();\n";
print BANKIMP "         (*classMap)[\"$bankName\"] = pointer;\n";
print BANKIMP "      };\n";
print BANKIMP "   virtual ~$regName()\n";
print BANKIMP "      {\n";
print BANKIMP "         PdbClassMap<PdbCalBank> *classMap = PdbClassMap<PdbCalBank>::instance();\n";
print BANKIMP "         if (classMap->find(\"$bankName\") != classMap->end())\n";
print BANKIMP "           {\n";
print BANKIMP "             delete (*classMap)[\"$bankName\"];\n";
print BANKIMP "             classMap->erase( \"$bankName\");\n";
print BANKIMP "           }\n";
print BANKIMP "      };\n";
print BANKIMP "};\n";print BANKIMP "\n";
$instName = join("", "reg", $bankName);
print BANKIMP "$regName $instName;\n";

close BANKIMP;

