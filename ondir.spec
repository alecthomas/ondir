Summary: ondir is a small program to automate tasks specific to certain directories. It works by executing scripts in directories when you enter and leave them.
Name: ondir
Version: 0.2.2
Release: 1
Copyright: GPL
Group: Development/Tools
Source: ondir-%{PACKAGE_VERSION}.tar.gz
BuildRoot: /var/tmp/%{name}-buildroot
Packager: Alec Thomas <alec@korn.ch>

%description
OnDir automatically executes scripts in directories when you traverse them at
the command line. This is useful for things like automatically changing your
umask when moving into your ~/public_html directory or adding a project
specific path to your PATH. 
%prep
%setup -q
%build
make PREFIX=/usr CONF=/etc/ondirrc
%install
make DESTDIR=$RPM_BUILD_ROOT PREFIX=/usr CONF=/etc/ondirrc install

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root)
%doc AUTHORS README COPYING INSTALL ChangeLog ondirrc.eg scripts.sh scripts.tcsh
/usr/bin/ondir
/usr/man/man1/ondir.1.gz
%changelog
* Sun Jul 13 2003 Alec Thomas <alec at korn period ch>
- Complete rewrite in C. No longer using automake/autoconf. Updated .spec accordingly.
* Mon Jun 10 2002 Alec Thomas <alec at korn period ch>
- Created build script and corresponding .spec file
