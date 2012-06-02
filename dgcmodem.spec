#
# Copyright (c) 2003-2010 Linuxant inc.
#
# NOTE: The use and distribution of this software is governed by the terms in
# the file LICENSE, which is included in the package. You must read this and
# agree to these terms before using or distributing this software.
#

%if "%{_target_distro}" == "custom"
%define _target_kernel %(eval uname -r)
%endif

# set _target_kernel to generic if it wasn't defined on the command-line
%if %{expand:%{?_target_kernel:0}%{!?_target_kernel:1}}
%define _target_kernel generic
%endif

%if %{expand:%{?_build_doc:0}%{!?_build_doc:1}}
%define _build_doc 0
%endif

%if %{expand:%{?_requires:0}%{!?_requires:1}}
%define _requires none
%endif

%if "%{_target_kernel}" == "generic"
%define ver      1.13
%define rel      1
%define automrecomp 1
%else
%define ver      1.13_k%(eval echo %{_target_kernel} | sed 's/-/_/g')
%define rel      1%{_target_distro}
%define automrecomp 0
%endif

%define cnxtdriver  dgc
%define cnxtdrvdsc  Conexant DGC USB modem driver
%define cnxttarget  dgc

%define scr_support 0
%define dmp_support 0
%define blam_support 0

# Note that this is NOT a relocatable package

%define cnxtetcdir /etc/dgcmodem
%define cnxtlibdir /usr/lib/dgcmodem

# Even though newer versions of RPM provide definitions for these,
# we must still accomodate the older ones
%define prefix   /usr
%define etcdir   /etc
%define libdir   %{prefix}/lib
%define sbindir  %{prefix}/sbin
%define bindir   %{prefix}/bin

Summary:   %{cnxtdrvdsc}
Name:      %{cnxttarget}modem
ExclusiveOS: Linux
%if "%{_target_kernel}" == "generic"
%if "%{cnxtdriver}" == "hsf"
ExclusiveArch: athlon i686 i586 i386 x86_64
%endif
%if "%{cnxtdriver}" == "hcf"
%if "%{_target_cpu}" == "ppc"
ExclusiveArch: ppc
%else
ExclusiveArch: athlon i686 i586 i486 i386
%endif
%endif
%else
%if "%{_target_distro}" != "custom"
ExclusiveArch: %{_target_cpu}
%endif
%endif
Version:   %ver
Release:   %rel
Vendor:	   Linuxant
License: Copyright (c) 2003-2010 Linuxant inc. All rights reserved.
Group:     System Environment/Kernel
Source:    %{cnxttarget}modem-1.13.tar.gz
%if %{_build_doc}
Source2:   100498D_RM_HxF_Released.pdf
%endif
URL:       http://www.linuxant.com/drivers/%{cnxtdriver}
BuildRoot: %{_tmppath}/%{cnxttarget}modem-%{PACKAGE_VERSION}-root
Packager:  Linuxant
Requires:  pciutils
%if %{automrecomp}
#Requires:  kernel-source
Requires:  gcc
%endif
%if "%{cnxtdriver}" == "hsf"
Requires:  perl
%endif
%if "%{cnxtdriver}" == "hsf" || "%{cnxtdriver}" == "hcf"
Conflicts: %{cnxtdriver}linmodem
%endif
%if "%{_requires}" != "none"
Requires: %{_requires}
%endif
Autoreq:   0

%if %{_build_doc}
%package doc
Group:     Documentation
Summary:   Documentation for %{cnxtdrvdsc}
BuildArch: noarch
ExclusiveArch: noarch
%endif

%description
%{cnxtdrvdsc}

Copyright (c) 2007-2010 Linuxant inc.

1.  General Public License. This program is free software, and may
be redistributed or modified subject to the terms of the GNU General
Public License (version 2) as specified in the file COPYING included
with this package.

2.   Disclaimer of Warranties. LINUXANT AND OTHER CONTRIBUTORS MAKE NO
REPRESENTATION ABOUT THE SUITABILITY OF THIS SOFTWARE FOR ANY PURPOSE.
IT IS PROVIDED "AS IS" WITHOUT EXPRESS OR IMPLIED WARRANTIES OF ANY KIND.
LINUXANT AND OTHER CONTRIBUTORS DISCLAIMS ALL WARRANTIES WITH REGARD TO
THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE, GOOD TITLE AND AGAINST INFRINGEMENT.

This software has not been formally tested, and there is no guarantee that
it is free of errors including, but not limited to, bugs, defects,
interrupted operation, or unexpected results. Any use of this software is
at user's own risk.

3.   No Liability.

(a) Linuxant or contributors shall not be responsible for any loss or
damage to user, or any third parties for any reason whatsoever, and
LINUXANT OR CONTRIBUTORS SHALL NOT BE LIABLE FOR ANY ACTUAL, DIRECT,
INDIRECT, SPECIAL, PUNITIVE, INCIDENTAL, OR CONSEQUENTIAL
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED, WHETHER IN CONTRACT, STRICT OR OTHER LEGAL THEORY OF
LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY
WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
OF SUCH DAMAGE.

(b) User agrees to hold Linuxant and contributors harmless from any
liability, loss, cost, damage or expense, including attorney's fees,
as a result of any claims which may be made by any person, including
but not limited to User, its agents and employees, its customers, or
any third parties that arise out of or result from the manufacture,
delivery, actual or alleged ownership, performance, use, operation
or possession of the software furnished hereunder, whether such claims
are based on negligence, breach of contract, absolute liability or any
other legal theory.

4.   Notices. User hereby agrees not to remove, alter or destroy any
copyright, trademark, credits, other proprietary notices or confidential
legends placed upon, contained within or associated with the Software,
and shall include all such unaltered copyright, trademark, credits,
other proprietary notices or confidential legends on or in every copy of
the Software.

%if %{_build_doc}
%description doc
This package contains the documentation for the %{cnxtdrvdsc}.
%endif

%prep
%setup -q -n %{cnxttarget}modem-1.13

%build

%if %{_build_doc}
if [ -f %{SOURCE2} ]; then cp %{SOURCE2} .; else true; fi
%else
make --quiet --no-print-directory all
%endif

%if "%{_target_kernel}" == "generic"

%else

MODS_DIR=binaries/linux-%{_target_kernel}
UNAMER=`uname -r`

# Figure out if we should add -SMP at the end of CNXT_MODS_DIR. We should only add it if the
# kernel was compiled with SMP and the word SMP doesn't appear in the kernel version. This
# is expected by dldrconfig.
case "%{_target_kernel}" in
*[Ss][Mm][Pp]*)
	SMPSUFFIX=""
	;;
*)
	SMPSUFFIX="-SMP"
	;;
esac

case "%{_target_distro}" in
rh | fdr | fs)
	CONFIGFILE="%{_distro_kernels}/%{_target_distro}/linux-%{_target_kernel}.%{_target_cpu}/.config"
	;;
custom)
	CONFIGFILE="/lib/modules/${UNAMER}/build/.config"
	;;
*)
	CONFIGFILE="%{_distro_kernels}/%{_target_distro}/linux-%{_target_kernel}/.config"
	;;
esac

if [ -e "${CONFIGFILE}" ] && grep -q '^CONFIG_SMP=y$' "${CONFIGFILE}"; then
	MODS_DIR="${MODS_DIR}${SMPSUFFIX}"
fi

%if "%{_target_distro}" == "rh"
(
	if cd modules; then
		case "%{_target_kernel}" in
		*.[Ee][Ll]*)
			make --quiet --no-print-directory CNXT_KERNELSRC=%{_distro_kernels}/%{_target_distro}/linux-%{_target_kernel}.%{_target_cpu} DISTRO_CFLAGS="-D__MODULE_KERNEL_%{_target_cpu}=1" CNXT_MODS_DIR="${MODS_DIR}" clean all modules_install || exit $?
			;;
		*)
			make --quiet --no-print-directory CNXT_KERNELSRC=%{_distro_kernels}/%{_target_distro}/linux-%{_target_kernel} DISTRO_CFLAGS="-D__BOOT_KERNEL_H_ -D__BOOT_KERNEL_ENTERPRISE=0 -D__BOOT_KERNEL_SMP=0 -D__BOOT_KERNEL_UP=1 -D__MODULE_KERNEL_%{_target_cpu}=1" CNXT_MODS_DIR="${MODS_DIR}" clean all modules_install || exit $?
			;;
		esac
	fi
) || exit $?
%endif

%if "%{_target_distro}" == "fdr"
(
	if cd modules; then
		case "%{_target_kernel}" in
		2.4*)
			make --quiet --no-print-directory CNXT_KERNELSRC=%{_distro_kernels}/%{_target_distro}/linux-%{_target_kernel} DISTRO_CFLAGS="-D__BOOT_KERNEL_H_ -D__BOOT_KERNEL_ENTERPRISE=0 -D__BOOT_KERNEL_SMP=0 -D__BOOT_KERNEL_UP=1 -D__MODULE_KERNEL_%{_target_cpu}=1" CNXT_MODS_DIR="${MODS_DIR}" clean all modules_install || exit $?
			;;
		*)
			make --quiet --no-print-directory CNXT_KERNELSRC=%{_distro_kernels}/%{_target_distro}/linux-%{_target_kernel}.%{_target_cpu} DISTRO_CFLAGS="" CNXT_MODS_DIR="${MODS_DIR}" clean all modules_install || exit $?
			;;
		esac
	fi
) || exit $?
%endif

%if "%{_target_distro}" == "fs"
(
	if cd modules; then
		case "%{_target_kernel}" in
		2.4*)
			make --quiet --no-print-directory CNXT_KERNELSRC=%{_distro_kernels}/%{_target_distro}/linux-%{_target_kernel} DISTRO_CFLAGS="-D__BOOT_KERNEL_H_ -D__BOOT_KERNEL_ENTERPRISE=0 -D__BOOT_KERNEL_SMP=0 -D__BOOT_KERNEL_UP=1 -D__MODULE_KERNEL_%{_target_cpu}=1" CNXT_MODS_DIR="${MODS_DIR}" clean all modules_install || exit $?
			;;
		*)
			make --quiet --no-print-directory CNXT_KERNELSRC=%{_distro_kernels}/%{_target_distro}/linux-%{_target_kernel}.%{_target_cpu} DISTRO_CFLAGS="" CNXT_MODS_DIR="${MODS_DIR}" clean all modules_install || exit $?
			;;
		esac
	fi
) || exit $?
%endif

%if "%{_target_distro}" == "mdk"
(
	if cd modules; then
		make --quiet --no-print-directory CNXT_KERNELSRC=%{_distro_kernels}/%{_target_distro}/linux-%{_target_kernel} DISTRO_CFLAGS="-D__BOOT_KERNEL_H_ -D__BOOT_KERNEL_ENTERPRISE=0 -D__BOOT_KERNEL_SMP=0 -D__BOOT_KERNEL_UP=1 -D__MODULE_KERNEL_%{_target_cpu}=1" CNXT_MODS_DIR="${MODS_DIR}" clean all modules_install || exit $?
	fi
) || exit $?
%endif

%if "%{_target_distro}" == "mdv"
(
	if cd modules; then
		make --quiet --no-print-directory CNXT_KERNELSRC=%{_distro_kernels}/%{_target_distro}/linux-%{_target_kernel} DISTRO_CFLAGS="" CNXT_MODS_DIR="${MODS_DIR}" clean all modules_install || exit $?
	fi
) || exit $?
%endif

%if "%{_target_distro}" == "suse"
(
	if cd modules; then
		make --quiet --no-print-directory CNXT_KERNELSRC=%{_distro_kernels}/%{_target_distro}/linux-%{_target_kernel} CNXT_MODS_DIR="${MODS_DIR}" clean all modules_install || exit $?
	fi
) || exit $?
%endif

%if "%{_target_distro}" == "jds"
(
	if cd modules; then
		make --quiet --no-print-directory CNXT_KERNELSRC=%{_distro_kernels}/%{_target_distro}/linux-%{_target_kernel} CNXT_MODS_DIR="${MODS_DIR}" clean all modules_install || exit $?
	fi
) || exit $?
%endif

%if "%{_target_distro}" == "turbo"
(
	if cd modules; then
		make --quiet --no-print-directory CNXT_KERNELSRC=%{_distro_kernels}/%{_target_distro}/linux-%{_target_kernel} DISTRO_CFLAGS="-D__BOOT_KERNEL_H_ -D__BOOT_KERNEL_SMP=0 -D__BOOT_KERNEL_SMP64G=0 -D__BOOT_KERNEL_UP=1 -D__BOOT_KERNEL_BOOT=0 -D__MODULE_KERNEL_%{_target_cpu}=1" CNXT_MODS_DIR="${MODS_DIR}" clean all modules_install || exit $?
	fi
) || exit $?
%endif

%if "%{_target_distro}" == "redflag"
(
	if cd modules; then
		make --quiet --no-print-directory CNXT_KERNELSRC=%{_distro_kernels}/%{_target_distro}/linux-%{_target_kernel} DISTRO_CFLAGS="" CNXT_MODS_DIR="${MODS_DIR}" clean all modules_install || exit $?
	fi
) || exit $?
%endif

%if "%{_target_distro}" == "custom"
(
	if cd modules; then
		make --quiet --no-print-directory CNXT_KERNELSRC=/lib/modules/${UNAMER}/build DISTRO_CFLAGS="" CNXT_MODS_DIR="${MODS_DIR}" clean all modules_install || exit $?
	fi
) || exit $?
%endif

%endif

%install
rm -rf $RPM_BUILD_ROOT

make --quiet --no-print-directory all
make --quiet --no-print-directory ROOT=$RPM_BUILD_ROOT install

if [ -d $RPM_BUILD_ROOT%{cnxtetcdir}/nvm ]; then
( cd $RPM_BUILD_ROOT%{cnxtetcdir}/nvm && cd .. && tar czf nvm.tar.gz nvm && rm -rf nvm/*)
fi

echo "RPM" > "$RPM_BUILD_ROOT/%{cnxtetcdir}/package"

%if "%{_target_kernel}" != "generic"
find "$RPM_BUILD_ROOT/%{cnxtlibdir}" \( -name '*.[chO]' -o -name 'Makefile' -o -name '*.mak' -o -name '*.sh' \) -exec rm -f {} \;
%endif

%clean
rm -rf $RPM_BUILD_ROOT

%files

%defattr(0555, root, root, 755)

%{sbindir}/%{cnxttarget}config
%{sbindir}/%{cnxttarget}stop
%{sbindir}/%{cnxttarget}modconflicts
%{cnxtlibdir}/rc%{cnxttarget}
%if "%{cnxtdriver}" == "hsf" || "%{cnxtdriver}" == "dgc"
%{sbindir}/%{cnxttarget}dcpd
%endif

%if %{scr_support}
%{sbindir}/%{cnxttarget}scr
%{bindir}/qtmodemon
%endif
%if %{dmp_support}
%{sbindir}/%{cnxttarget}dmp
%endif
%if %{blam_support}
%{sbindir}/%{cnxttarget}diag
%endif

%defattr(0444, root, root, 755)
%dir %{cnxtetcdir}
%if "%{cnxtdriver}" != "dgc"
%dir %{cnxtetcdir}/nvm
%{cnxtetcdir}/nvm.tar.gz
%endif
%{cnxtetcdir}/package
%dir %{cnxtlibdir}
%doc %{cnxtlibdir}/LICENSE
%dir %{cnxtlibdir}/modules
%dir %{cnxtlibdir}/modules/GPL
%doc %{cnxtlibdir}/modules/GPL/COPYING
%{cnxtlibdir}/modules/binaries

%if %{automrecomp}
%config %{cnxtlibdir}/config.mak
%{cnxtlibdir}/modules/imported
%attr(755, root, root) %{cnxtlibdir}/modules/*.sh
%{cnxtlibdir}/modules/Makefile
%{cnxtlibdir}/modules/*.c
%if "%{cnxtdriver}" != "dgc"
%{cnxtlibdir}/modules/GPL/*.c
%endif
%{cnxtlibdir}/modules/GPL/*.h
%if "%{cnxtdriver}" == "hsf"
%dir %{cnxtlibdir}/modules/GPL/hda
%{cnxtlibdir}/modules/GPL/hda/*.c
%{cnxtlibdir}/modules/GPL/hda/*.h
%{cnxtlibdir}/modules/GPL/hda/Makefile
%endif
%dir %{cnxtlibdir}/modules/include
%{cnxtlibdir}/modules/include/*.h
%endif

%doc BUGS CHANGES INSTALL LICENSE README
%if "%{cnxtdriver}" != "dgc"
%doc FAQ CREDITS
%endif

%pre
if [ "$1" = 1 ]; then
	if [ -e "%{cnxtetcdir}" ]; then
		echo "Removing old %{cnxtetcdir}"
		rm -rf "%{cnxtetcdir}"
	fi
	if [ -e "%{cnxtlibdir}" ]; then
		echo "Removing old %{cnxtlibdir}"
		rm -rf "%{cnxtlibdir}"
	fi
fi
exit 0

%post

if [ -f %{cnxtetcdir}/nvm.tar.gz ]; then
	( cd %{cnxtetcdir} && tar xzf nvm.tar.gz )
fi

#%if ! %{automrecomp}
CNXT_AUTOCONFIG=true
#%endif
if [ -n "${CNXT_AUTOCONFIG}" -a -z "${CNXT_NOAUTOCONFIG}" ]; then
	%{sbindir}/%{cnxttarget}config --auto
	ret=$?
	if [ "${ret}" -eq 123 ]; then
		ret=0
	fi
	if [ "${ret}" -eq 124 -a -n "${CNXT_NOWRONGKERNELFAIL}" ]; then
		ret=0
	fi
	exit ${ret}
else
	echo "To complete the installation and configuration of your modem,"
	echo "please run \"%{cnxttarget}config\" (or \"%{sbindir}/%{cnxttarget}config\")"
	exit 0
fi

%preun
%{sbindir}/%{cnxttarget}stop
if [ "$1" = 0 ]; then

	if [ -z "${CNXT_NOAUTOCONFIG}" ]; then
		%{sbindir}/%{cnxttarget}config --remove
	fi

	if [ -f %{cnxtetcdir}/nvm.tar.gz ]; then
		( cd %{cnxtetcdir} && rm -rf `tar tzf %{cnxtetcdir}/nvm.tar.gz | egrep '^nvm/[^/]+/?$'` )
	fi
else
	exit 0
fi

%if %{_build_doc}
%files doc
%doc *.pdf
%endif

# This must be last since the file CHANGES is automatically appended
%changelog

* Sun May 09 2010 -
	- Released dgcmodem-1.13.

* Sun May 09 2010 -
	- Improved compatibility with various kernels and distributions.

* Thu Mar 25 2010 -
	- Released dgcmodem-1.12.

* Thu Mar 25 2010 -
	- Added USB ID 17EF:7000 for Lenovo.

* Wed Oct 21 2009 -
	- Released dgcmodem-1.11.

* Wed Oct 21 2009 -
	- Improved compatibility with various kernels and distributions.

* Wed Apr 29 2009 -
	- Released dgcmodem-1.10.

* Wed Apr 29 2009 -
	- Improved compatibility with various kernels and distributions.

* Thu Dec 18 2008 -
	- Released dgcmodem-1.09.

* Thu Dec 18 2008 -
	- Improved compatibility with various kernels and distributions.

* Tue Sep 09 2008 -
	- Released dgcmodem-1.08.

* Tue Sep 09 2008 -
	- Improved compatibility with various kernels and distributions.

* Fri Jul 18 2008 -
	- Released dgcmodem-1.07.

* Fri Jul 18 2008 -
	- Improved compatibility with various kernels and distributions.

* Fri Jun 20 2008 -
	- Released dgcmodem-1.06.

* Fri Jun 20 2008 -
	- Improved compatibility with various kernels and distributions.

* Thu May 22 2008 -
	- Released dgcmodem-1.05.

* Thu May 22 2008 -
	- Improved compatibility with various kernels and distributions.

* Mon Mar 24 2008 -
	- Released dgcmodem-1.04.

* Mon Mar 24 2008 -
	- Improved compatibility with newer kernels and distributions.
	- Added PNPID for Harley2 USB.

* Fri Oct 19 2007 -
	- Released dgcmodem-1.03.

* Thu Oct 11 2007 -
	- Released dgcmodem-1.02.

* Thu Oct 11 2007 -
	- Added support for x86_64 architecture.
	- Improved compatibility with newer kernels and distributions.

* Tue Jun 05 2007 -
	- Released dgcmodem-1.01.

* Tue Jun 05 2007 -
	- Added PNPID for ACF Zoom Harley USB.

* Thu May 31 2007 -
	- Released dgcmodem-1.00.

* Mon May 07 2007 -
	- Released dgcmodem-0.91beta.

* Mon May 07 2007 -
	- Added additional PnP IDs.

* Mon Apr 30 2007 -
	- Released dgcmodem-0.90beta.
