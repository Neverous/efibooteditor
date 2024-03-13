Name:           efibooteditor
Version:        1.4.0
Release:        1%{?dist}
Summary:        Boot Editor for (U)EFI based systems

License:        GPLv3-or-later
URL:            https://github.com/Neverous/efibooteditor
Source0:        https://github.com/Neverous/efibooteditor/archive/refs/tags/v%{version}.tar.gz

BuildRequires:  cmake
BuildRequires:  cmake(Qt5Core)
BuildRequires:	cmake(Qt5Tools)
BuildRequires:  pkgconfig(efivar)

Requires:       efivar

%description
%{summary}.

%prep
%autosetup -n %{name}-%{version}


%build
%cmake
%cmake_build


%install
%cmake_install


%files
%doc doc/*
%license LICENSE.txt
%{_bindir}/%{name}
%{_datadir}/applications/EFIBootEditor.desktop
%{_datadir}/polkit-1/actions/org.x.%{name}.policy
%{_metainfodir}/EFIBootEditor.metainfo.xml



%changelog
* Tue Mar 28 2023 - 1.2.0-beta.2-1
- Updated version
* Sun Jan 22 2023 Justin Zobel <justin@1707.io> - 1.1.5-1
- Initial Version
