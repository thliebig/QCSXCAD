#
# spec file for package [spectemplate]
#
# Copyright (c) 2010 SUSE LINUX Products GmbH, Nuernberg, Germany.
#
# All modifications and additions to the file contributed by third parties
# remain the property of their copyright owners, unless otherwise agreed
# upon. The license for this file, and modifications and additions to the
# file, is the same license as for the pristine package itself (unless the
# license for the pristine package is not an Open Source License, in which
# case the license is the MIT License). An "Open Source License" is a
# license that conforms to the Open Source Definition (Version 1.9)
# published by the Open Source Initiative.

# Please submit bugfixes or comments via http://bugs.opensuse.org/
#

# norootforbuild

Name:           QCSXCAD
Version:        0.3.0
Release:        1
Summary:        Qt bindings for CSXCAD
Group:          Development/Languages/C and C++
License:        LGPLv3
URL:            http://www.openems.de
Source0:        %{name}-%{version}.tar.bz2
Patch0:         fedora17.diff
# BuildArch:      noarch
BuildRoot:      %_tmppath/%name-%version-build

BuildRequires:  libqt4-devel gcc-c++ tinyxml-devel vtk-devel vtk-qt CSXCAD-devel
#Requires:


# determine qt4 qmake executable
%if 0%{?fedora}
    %global qmake qmake-qt4
%else
    %global qmake qmake
%endif



%description
Qt bindings for CSXCAD.

%package        -n lib%{name}0
Summary:        Shared Library for %{name}
Group:          Development/Languages/C and C++

%description    -n lib%{name}0
The lib%{name}0 package contains the shared library.

%package        devel
Summary:        Development files for %{name}
Group:          Development/Languages/C and C++
Requires:       %{name} = %{version}-%{release}
Requires:       lib%{name}0 = %{version}-%{release}

%description    devel
The %{name}-devel package contains libraries and header files for
developing applications that use %{name}.


%prep
%setup -q
%if 0%{?fedora} >= 17
%patch0 -p1
%endif

%build
%qmake QMAKE_CFLAGS="%optflags" QMAKE_CXXFLAGS="%optflags" LIB_SUFFIX="$(echo %_lib | cut -b4-)"
make %{?_smp_mflags}


%install
make INSTALL_ROOT=%{buildroot} install
find %{buildroot} -name '*.la' -exec rm -f {} ';'


%clean
rm -rf %{buildroot}


%post -n lib%{name}0 -p /sbin/ldconfig

%postun -n lib%{name}0 -p /sbin/ldconfig


%files
%defattr(-,root,root)
%doc COPYING


%files -n lib%{name}0
%defattr(-,root,root)
%{_libdir}/*.so.*


%files devel
%defattr(-,root,root)
%{_includedir}/*
%{_libdir}/*.so


%changelog
* Sun Jun 17 2012 Sebastian Held <sebastian.held@gmx.de> - 0.3.0-1
- new upstream version
* Thu Dec 29 2011 Sebastian Held <sebastian.held@gmx.de> - 0.1.3-3
- new upstream version
* Wed Dec 28 2011 Sebastian Held <sebastian.held@gmx.de> - 0.1.3-2
- Fedora 16 build support
* Sun Dec 04 2011 Sebastian Held <sebastian.held@gmx.de> - 0.1.3-1
- initial version
