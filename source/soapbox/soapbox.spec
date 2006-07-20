%define _libdir /lib

Summary: A library to restrict processes from tampering with directories/files.
Name: soapbox
Version: 0.3.0
Release: 0
License: GPL
Group: System Environment/Libraries
URL: http://dag.wieers.com/home-made/soapbox/

Packager: Dag Wieers <dag@wieers.com>
Vendor: Dag Apt Repository, http://dag.wieers.com/home-made/apt/

Source: http://dag.wieers.com/home-made/%{name}/%{name}-%{version}.tar.bz2
BuildRoot: %{_tmppath}/root-%{name}-%{version}
Prefix: %{_prefix}

%description
Soapbox allows to restrict specific processes to write only to those
places you want files to be changed. Read-access however is still
based on file-permissions.

By preloading the Soapbox library, you can run programs as root and
still be safe that they don't mess up your system.

%prep
%setup

%build
%{__make} %{?_smp_mflags}

%install
%{__rm} -rf %{buildroot}
%makeinstall

%clean
%{__rm} -rf %{buildroot}

%files
%defattr(-, root, root, 0755)
%doc AUTHORS BUGS ChangeLog COPYING COPYRIGHT README THANKS TODO
%{_libdir}/*
%{_bindir}/*

%changelog
* Mon Apr 21 2003 Dag Wieers <dag@wieers.com> - 0.2.1-0
- Updated to release 0.2.1.

* Sat Apr 19 2003 Dag Wieers <dag@wieers.com> - 0.2.0-0
- Updated to release 0.2.0.

* Wed Mar 26 2003 Dag Wieers <dag@wieers.com> - 0.1.2-0
- Updated to release 0.1.2.

* Tue Mar 25 2003 Dag Wieers <dag@wieers.com> - 0.1.1-0
- Updated to release 0.1.1.

* Sun Mar 23 2003 Dag Wieers <dag@wieers.com> - 0.1.0-0
- Updated to release 0.1.0.

* Tue Mar 18 2003 Dag Wieers <dag@wieers.com> - 0.0.12-0
- Updated to release 0.0.12.

* Mon Mar 17 2003 Dag Wieers <dag@wieers.com> - 0.0.11-0
- Updated to release 0.0.11.

* Sun Mar 16 2003 Dag Wieers <dag@wieers.com> - 0.0.10-0
- Updated to release 0.0.10.

* Sat Mar 15 2003 Dag Wieers <dag@wieers.com> - 0.0.7-0
- Updated to release 0.0.7.

* Fri Mar 14 2003 Dag Wieers <dag@wieers.com> - 0.0.3-0
- Initial package. (using dar)
