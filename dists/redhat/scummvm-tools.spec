#------------------------------------------------------------------------------
#   scummvm-tools.spec
#       This SPEC file controls the building of ScummVM Tools RPM packages.
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
#   Prologue information
#------------------------------------------------------------------------------
Name		: scummvm-tools
Version		: 1.2.0pre
Release		: 1
Summary		: ScummVM-related tools
Group		: Interpreters
License		: GPL

Url             : http://www.scummvm.org

Source		: %{name}-%{version}.tar.bz2
Source1		: libmad-0.15.1b.tar.bz2
BuildRoot	: %{_tmppath}/%{name}-%{version}-root

BuildRequires	: zlib-devel
BuildRequires	: wxGTK-devel
#------------------------------------------------------------------------------
#   Description
#------------------------------------------------------------------------------
%description
Tools for compressing ScummVM datafiles and other related tools.

#------------------------------------------------------------------------------
#   install scripts
#------------------------------------------------------------------------------
%prep
%setup -q -a 1 -n scummvm-tools-%{version}

%build
(cd libmad-0.15.1b; grep -v 'force-\(mem\|addr\)' configure > configure.new; mv -f configure.new configure; chmod 700 configure; ./configure --enable-static --disable-shared --prefix=%{_builddir}/scummvm-%{version}/tmp; make; make install)
./configure --with-mad-prefix=%{_builddir}/scummvm-%{version}/tmp
make
echo -e "                This script is installed as\n                "%{_datadir}/scummvm-tools/convert_dxa.sh.sample >> README

%install
install -m755 -d %{buildroot}%{_bindir}
install -m755 -D create_sjisfnt %{buildroot}%{_bindir}
install -m755 -D scummvm-tools{,-cli} %{buildroot}%{_bindir}
install -m755 -D de{cine,gob,kyra,riven,scumm,sword2} %{buildroot}%{_bindir}
install -m755 -D {construct,extract}_mohawk %{buildroot}%{_bindir}
install -m644 -D convert_dxa.sh %{buildroot}%{_datadir}/scummvm-tools/convert_dxa.sh.sample

%clean
rm -Rf ${RPM_BUILD_ROOT}

#------------------------------------------------------------------------------
#   Files listing.
#------------------------------------------------------------------------------
%files
%doc README COPYING
%attr(0755,root,root)%{_bindir}/scummvm*
%attr(0755,root,root)%{_bindir}/create_sjisfnt
%attr(0755,root,root)%{_bindir}/de*
%attr(0755,root,root)%{_bindir}/extract_*
%attr(0755,root,root)%{_bindir}/construct_*
%attr(0644,root,root)%{_datadir}/scummvm-tools/convert_dxa.sh.sample

#------------------------------------------------------------------------------
#   Change Log
#------------------------------------------------------------------------------
%changelog
* Sat Mar 26 2005 (0.7.1)
  - first tools package
