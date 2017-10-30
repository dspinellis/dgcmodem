/*
 * Copyright (c) 2007-2008 Linuxant inc.
 * 
 * 1.  General Public License. This program is free software, and may
 * be redistributed or modified subject to the terms of the GNU General
 * Public License (version 2) or the GNU Lesser General Public License,
 * or (at your option) any later versions ("Open Source" code). You may
 * obtain a copy of the GNU General Public License at
 * http://www.fsf.org/copyleft/gpl.html and a copy of the GNU Lesser
 * General Public License at http://www.fsf.org/copyleft/less.html,
 * or you may alternatively write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA.
 * 
 * 2.   Disclaimer of Warranties. LINUXANT AND OTHER CONTRIBUTORS MAKE NO
 * REPRESENTATION ABOUT THE SUITABILITY OF THIS SOFTWARE FOR ANY PURPOSE.
 * IT IS PROVIDED "AS IS" WITHOUT EXPRESS OR IMPLIED WARRANTIES OF ANY KIND.
 * LINUXANT AND OTHER CONTRIBUTORS DISCLAIMS ALL WARRANTIES WITH REGARD TO
 * THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE, GOOD TITLE AND AGAINST INFRINGEMENT.
 * 
 * This software has not been formally tested, and there is no guarantee that
 * it is free of errors including, but not limited to, bugs, defects,
 * interrupted operation, or unexpected results. Any use of this software is
 * at user's own risk.
 * 
 * 3.   No Liability.
 * 
 * (a) Linuxant or contributors shall not be responsible for any loss or
 * damage to user, or any third parties for any reason whatsoever, and
 * LINUXANT OR CONTRIBUTORS SHALL NOT BE LIABLE FOR ANY ACTUAL, DIRECT,
 * INDIRECT, SPECIAL, PUNITIVE, INCIDENTAL, OR CONSEQUENTIAL
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED, WHETHER IN CONTRACT, STRICT OR OTHER LEGAL THEORY OF
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY
 * WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 * 
 * (b) User agrees to hold Linuxant and contributors harmless from any
 * liability, loss, cost, damage or expense, including attorney's fees,
 * as a result of any claims which may be made by any person, including
 * but not limited to User, its agents and employees, its customers, or
 * any third parties that arise out of or result from the manufacture,
 * delivery, actual or alleged ownership, performance, use, operation
 * or possession of the software furnished hereunder, whether such claims
 * are based on negligence, breach of contract, absolute liability or any
 * other legal theory.
 * 
 * 4.   Notices. User hereby agrees not to remove, alter or destroy any
 * copyright, trademark, credits, other proprietary notices or confidential
 * legends placed upon, contained within or associated with the Software,
 * and shall include all such unaltered copyright, trademark, credits,
 * other proprietary notices or confidential legends on or in every copy of
 * the Software.
 * 
 */
#include "oscompat.h"
#include "osresour_ex.h"
#include "oslinux.h"
#include "dcp.h"

#include <linux/usb.h>

/* table of devices that work with this driver */
static struct usb_device_id dgcusbdcp_tbl [] = {
	// Harley, Controller, SmartDAA, Chipset Default
	// Data/Fax
	{ USB_DEVICE(0x572, 0x1320) },

	// Harley, Controller, SmartDAA
	// Data/Fax/RTAM
	{ USB_DEVICE(0x572, 0x1321) },

	// Harley, Controller, SmartDAA
	// Speakerphone
	{ USB_DEVICE(0x572, 0x1322) },

	// Harley, Controller, Discrete DAA
	// Data/Fax
	{ USB_DEVICE(0x572, 0x1323) },

	// Dell-Europa, Harley, SmartDAA, SmartCP
	// Data/Fax
	{ USB_DEVICE(0x572, 0x1324) },

	// Harley, USB ACF, SmartDAA, PreSigned
	// Data/Fax, CID type 1, MOH, SmartCP
	{ USB_DEVICE(0x572, 0x1328) },

	// Harley, USB ACF, SmartDAA, PreSigned
	// Data/Fax/RTAM, CID type 1, MOH, SmartCP
	{ USB_DEVICE(0x572, 0x1329) },

	// Harley2
	{ USB_DEVICE(0x572, 0x1340) },

	// Harley, USB ACF, Zoom
	{ USB_DEVICE(0x803, 0x3095) },

	{ USB_DEVICE(0x572, 0x1348) },
	{ USB_DEVICE(0x572, 0x1349) },

	// Harley, Lenovo
	{ USB_DEVICE(0x17EF, 0x7000) },

	{ }
};

MODULE_DEVICE_TABLE(usb, dgcusbdcp_tbl);

#ifndef CNXTHW_DECL_MODULE_DEVICE_TABLE_ONLY
MODULE_AUTHOR("Copyright (C) 2007 Linuxant inc.");
MODULE_DESCRIPTION("Conexant diagnostic channel driver");
MODULE_LICENSE("GPL");
MODULE_INFO(supported, "yes");
#endif

#include <linux/slab.h>


typedef struct tagUSBOSHAL {
	struct usb_device *pUsbDevice;
	struct urb *rx_urb;
	int DiagInPipe, DiagOutPipe;
	u8 rx_buf[256];
	u8 dx_buf[256];
	BOOL esc;
	HANDLE hDcp;
} USBOSHAL, *PUSBOSHAL;

#undef dbg
#define dbg(format, arg...) { printk(KERN_DEBUG __FILE__ ": " format "\n" , ## arg); }

#ifndef info
#define info(format, arg...) printk(KERN_INFO KBUILD_MODNAME ": " format "\n" , ## arg)
#endif

#ifndef warn
#define warn(format, arg...) printk(KERN_WARNING KBUILD_MODNAME ": " format "\n" , ## arg)
#endif

#ifndef err
#define err(format, arg...) { printk(KERN_ERR __FILE__ ": " format "\n" , ## arg); }
#endif

static int dgcusbdcp_probe(struct usb_interface *intf, const struct usb_device_id *id);
static void dgcusbdcp_disconnect(struct usb_interface *intf);

static struct usb_driver dgcusbdcp_driver = {
#if defined(FOUND_USB_DRIVER_OWNER)
    .owner          = THIS_MODULE,
#endif
    .name           = "dcgusbdcp",
    .id_table       = dgcusbdcp_tbl,
    .probe          = dgcusbdcp_probe,
    .disconnect     = dgcusbdcp_disconnect,
};

#define CFGDESC(x) desc.x

static int
dgcUsbWrite(PUSBOSHAL pUsbOsHal, unsigned char *data, size_t len)
{
	int retval;
	unsigned char *buffer;
	int actual_length = 0;

	dbg ("%s: data=%p len=%d", __FUNCTION__, data, len);

	if (!(buffer = (unsigned char *)__get_free_page(GFP_KERNEL))) {
		err("could not get a buffer for dgcUsbWrite (len=%d)\n", (int)len);
		return -ENOMEM;
	}

	memcpy (buffer, data, len);

	retval = usb_bulk_msg(pUsbOsHal->pUsbDevice, pUsbOsHal->DiagOutPipe, buffer, len, &actual_length, 2000);

	if (retval < 0) {
		err("%s: usb_bulk_msg failed (err=%d)\n", __FUNCTION__, retval);
		free_page ((unsigned long) buffer);
		return -ENOMEM;
	}

	free_page((unsigned long)buffer);

	return len;
}

static void
dgcUsbRxDone(struct urb *urb
#if defined(FOUND_USB_COMPLETE_PT_REGS)
    , struct pt_regs *regs
#endif
)
{
	PUSBOSHAL pUsbOsHal = (PUSBOSHAL)urb->context;
	int status;
	int i;
	u8 *s, *d;
	u16 *w;

	if (urb->status) {
		if(urb->status != -ESHUTDOWN)
			err("%s: status %d, length %d", __FUNCTION__, urb->status, urb->actual_length);
		return;
	}

	//dbg("%s: length %d", __FUNCTION__, urb->actual_length);

	s = urb->transfer_buffer;
	d = pUsbOsHal->dx_buf;

	i = urb->actual_length;
	if(i > sizeof(pUsbOsHal->dx_buf)) {
		i = sizeof(pUsbOsHal->dx_buf);
	}

	while(i--) {
		if(pUsbOsHal->esc) {
			if(*s == 0x19) {
				*d++ = *s;
			} else {
				if((*s & 0xf ) == 1) {
					DcpSetVolume(pUsbOsHal->hDcp, (*s >> 4) & 7);
				} else
					printk(KERN_DEBUG"%s: unexpected EM 0x%x\n", __FUNCTION__, *s);
			}
			pUsbOsHal->esc = FALSE;
			s++;
			continue;
		}
		if(*s == 0x19) {
			pUsbOsHal->esc = TRUE;
			s++;
			continue;
		}
		*d++ = *s++;
	}

	// swap samples: (big-endian to little-endian)
	w = (u16*)pUsbOsHal->dx_buf;
	for(i = (d - pUsbOsHal->dx_buf) / sizeof(*w); i; i--, w++)
		*w = __swab16(*w);

	DcpCallback(pUsbOsHal->hDcp, pUsbOsHal->dx_buf, d - pUsbOsHal->dx_buf);

	urb->dev = pUsbOsHal->pUsbDevice;
	if ((status = usb_submit_urb(urb, GFP_ATOMIC)) < 0) {
		dbg("submit(rx_urb) status %d", status);
	}
}

static DEFINE_SPINLOCK(dgcusbdcp_lock);
static unsigned int dgcusbdcp_instance_bitmap;
#define DGCUSBDCP_MAX_INSTANCES (sizeof(dgcusbdcp_instance_bitmap)*8)

/* "Union Functional Descriptor" from CDC spec 5.2.3.X */
struct usb_cdc_union_desc {
	u8	bLength;
	u8	bDescriptorType;
	u8	bDescriptorSubType;

	u8	bMasterInterface0;
	u8	bSlaveInterface0;
	/* ... and there could be other slave interfaces */
} __attribute__ ((packed));

#ifndef USB_CDC_UNION_TYPE
#define USB_CDC_UNION_TYPE			0x06
#endif

static struct usb_cdc_union_desc fake_cdc_union_desc = {
	.bLength = sizeof(struct usb_cdc_union_desc),
	.bDescriptorType = USB_DT_CS_INTERFACE,
	.bDescriptorSubType = USB_CDC_UNION_TYPE,
	.bMasterInterface0 = 0,
	.bSlaveInterface0 = 1
};

static struct list_head udwa_list = LIST_HEAD_INIT(udwa_list);


typedef struct {
	struct list_head entry;

	unsigned char *orig_extra;
	int orig_extralen;

	unsigned char **orig_extra_p;
	int *orig_extralen_p;
} udwa_t;

static int dgcusbdcp_probe(struct usb_interface *intf,
	const struct usb_device_id *id)
{
	unsigned long flags;
    int i;
    struct usb_interface *pUsbDiagInterface;
    POS_DEVNODE pDevNode = NULL;
    PUSBOSHAL pUsbOsHal = NULL;
    struct usb_device *pUsbDevice = interface_to_usbdev(intf);
    __u8 ifnum = intf->altsetting->desc.bInterfaceNumber;

    dbg("%s: pUsbDevice=%p ifnum=%d id=%p altsetting=%p endpoint=%p", __FUNCTION__, pUsbDevice, ifnum, id, intf->altsetting, intf->altsetting->endpoint);
//    dbg("%s: NAME=%s %s", __FUNCTION__, pUsbDevice->dev.bus_id, pUsbDevice->dev.driver->name);

	if(pUsbDevice->descriptor.bNumConfigurations != 2) {
		err("%s: Wrong number of device configurations (%d)", __FUNCTION__, pUsbDevice->descriptor.bNumConfigurations);
		goto exit;
	}

	if(pUsbDevice->actconfig->CFGDESC(bConfigurationValue) != 2 && (ifnum == 0)) {
		dbg("%s: current config=%d, setting to 2...", __FUNCTION__, pUsbDevice->actconfig->CFGDESC(bConfigurationValue));
		i = usb_driver_set_configuration(pUsbDevice, 2);
		dbg("%s: usb_driver_set_configuration returned %d", __FUNCTION__, i);
		goto exit;
	}

	/* workaround: add fake union descriptor if missing */
	if(ifnum == 0 && intf->altsetting->extra && !intf->altsetting->extralen) {
		udwa_t *udwa;

		udwa = kmalloc(sizeof(*udwa), GFP_KERNEL);
		if(udwa) {
			memset(udwa, 0, sizeof(*udwa));

			warn("%s: adding union descriptor for cdc_acm", __FUNCTION__);

			spin_lock_irqsave(&dgcusbdcp_lock, flags);
			udwa->orig_extra_p = &intf->altsetting->extra;
			udwa->orig_extra = intf->altsetting->extra;
			udwa->orig_extralen_p = &intf->altsetting->extralen;

			intf->altsetting->extra = (char*)&fake_cdc_union_desc;
			intf->altsetting->extralen = sizeof(fake_cdc_union_desc);

			list_add(&udwa->entry, &udwa_list);
			spin_unlock_irqrestore(&dgcusbdcp_lock, flags);

	{
		char *argv[4], *envp[3];
		char cmdbuf[120];

		sprintf(cmdbuf, "(sleep 1; /sbin/modprobe --ignore-install cdc_acm) </dev/null >/dev/null 2>&1 &");

		envp [0] = "HOME=/";
		envp [1] = "PATH=/sbin:/bin:/usr/sbin:/usr/bin";
		envp [2] = 0;

		argv[0] = "/bin/sh";
		argv[1] = "-c";
		argv[2] = cmdbuf;
		argv[3] = 0;

		i = call_usermodehelper(argv[0], argv, envp, 1);
		if(i && (i != SIGTERM)) {
	    		printk(KERN_ERR"%s: %s returned %d\n", __FUNCTION__, argv[0], i);
		}
	}
		}
	}

	if(ifnum != 2) {
		dbg("%s: Wrong interface (%d, want 2)", __FUNCTION__, ifnum);
		goto exit;
	}

    if(pUsbDevice->actconfig->CFGDESC(bNumInterfaces) != 3) {
		dbg("%s: Wrong number of device interfaces (%d)", __FUNCTION__, pUsbDevice->actconfig->CFGDESC(bNumInterfaces));
		goto exit;
	}

    pUsbDiagInterface = usb_ifnum_to_if(pUsbDevice, 2);

    if(pUsbDiagInterface->num_altsetting != 1) {
		warn("DiagInterface has more than one alternate setting (%d)", pUsbDiagInterface->num_altsetting);
    }

    if(pUsbDiagInterface->altsetting->CFGDESC(bNumEndpoints) != 2) {
		err("Wrong number of endpoints (%d) for DiagInterface", pUsbDiagInterface->altsetting->CFGDESC(bNumEndpoints));
		goto exit;
	}

    pUsbOsHal = kmalloc(sizeof(USBOSHAL), GFP_KERNEL);
	if (!pUsbOsHal) {
		err ("Out of memory");
		goto exit;
    }
    memset(pUsbOsHal, 0, sizeof(USBOSHAL));

    for(i=0; i < pUsbDiagInterface->altsetting->CFGDESC(bNumEndpoints); i++) {
		if (pUsbDiagInterface->altsetting->endpoint[i].CFGDESC(bEndpointAddress) == 0x83) {
			pUsbOsHal->DiagInPipe = usb_rcvbulkpipe (pUsbDevice, pUsbDiagInterface->altsetting->endpoint[i].CFGDESC(bEndpointAddress) & USB_ENDPOINT_NUMBER_MASK);
		} else if (pUsbDiagInterface->altsetting->endpoint[i].CFGDESC(bEndpointAddress) == 0x03) {
			pUsbOsHal->DiagOutPipe = usb_sndbulkpipe (pUsbDevice, pUsbDiagInterface->altsetting->endpoint[i].CFGDESC(bEndpointAddress) & USB_ENDPOINT_NUMBER_MASK);
		}
	}

    if(!pUsbOsHal->DiagInPipe || !pUsbOsHal->DiagOutPipe) {
		err("Missing endpoint(s)");
		goto exit;
	}

	pUsbOsHal->pUsbDevice = pUsbDevice;

	pDevNode = kmalloc(sizeof(*pDevNode), GFP_KERNEL);
	if(!pDevNode) {
		err ("Out of memory");
		goto exit;
	}

    memset(pDevNode, 0, sizeof(*pDevNode));
	pDevNode->hwDev = pUsbOsHal;
	pDevNode->hwDevLink = &pUsbDevice->dev;
	pDevNode->hwModule = THIS_MODULE;

	pDevNode->hwInstNum = -1;

	spin_lock_irqsave(&dgcusbdcp_lock, flags);
	for(i = 0; (i < DGCUSBDCP_MAX_INSTANCES) && (dgcusbdcp_instance_bitmap & (1 << i)); i++);
	if(i == DGCUSBDCP_MAX_INSTANCES) {
		spin_unlock_irqrestore(&dgcusbdcp_lock, flags);
		err("No free instances");
		goto exit;
	}
	dgcusbdcp_instance_bitmap |= (1 << i);
	pDevNode->hwInstNum = i;
	spin_unlock_irqrestore(&dgcusbdcp_lock, flags);

	pUsbOsHal->hDcp = DcpCreate(pDevNode);
	if(!pUsbOsHal->hDcp) {
		err ("cannot create DCP instance");
		goto exit;
	}

	pUsbOsHal->rx_urb = usb_alloc_urb(0, GFP_KERNEL);
	if(!pUsbOsHal->rx_urb) {
		err ("cannot allocate rx_urb");
		goto exit;
	}

	usb_fill_bulk_urb(pUsbOsHal->rx_urb, pUsbDevice, pUsbOsHal->DiagInPipe,
			pUsbOsHal->rx_buf, sizeof(pUsbOsHal->rx_buf), dgcUsbRxDone, pUsbOsHal);

	if ((i = usb_submit_urb(pUsbOsHal->rx_urb , GFP_ATOMIC)) < 0) {
		err("submit(rx_urb) status %d", i);
	}

	DcpSetVolume(pUsbOsHal->hDcp, 1);

	i = dgcUsbWrite(pUsbOsHal, "\x19\x01", 2);
	if(i != 2) {
		err ("cannot enable DCP mode (%d)", i);
		goto exit;
	}

    usb_set_intfdata (intf, pDevNode);
    return 0;

exit:

	if(pUsbOsHal) {
		if(pUsbOsHal->rx_urb) {
			usb_kill_urb(pUsbOsHal->rx_urb);
			usb_free_urb(pUsbOsHal->rx_urb);
		}

		if(pUsbOsHal->hDcp) {
			DcpDestroy(pUsbOsHal->hDcp);
		}
	}

	if(pDevNode) {
		if(pDevNode->hwInstNum != -1) {
			spin_lock_irqsave(&dgcusbdcp_lock, flags);
			dgcusbdcp_instance_bitmap &= ~(1 << pDevNode->hwInstNum);
			spin_unlock_irqrestore(&dgcusbdcp_lock, flags);
		}

		kfree(pDevNode);
	}
	if(pUsbOsHal) {
		kfree(pUsbOsHal);
	}
    return -ENODEV;
}

static void revert_udwa(void)
{
	struct list_head *tmp;
	struct list_head *tmp2;
	udwa_t *udwa;
	unsigned long flags;

	spin_lock_irqsave(&dgcusbdcp_lock, flags);

	list_for_each_safe(tmp, tmp2, &udwa_list) {
		list_del(tmp);

		udwa = list_entry(tmp, udwa_t, entry); 

		if((*udwa->orig_extra_p == (unsigned char*)&fake_cdc_union_desc) && (*udwa->orig_extralen_p == sizeof(fake_cdc_union_desc))) {
			*udwa->orig_extralen_p = 0;
			*udwa->orig_extra_p = udwa->orig_extra;
			spin_unlock_irqrestore(&dgcusbdcp_lock, flags);

			warn("reverted union descriptor for cdc_acm");

		} else
			spin_unlock_irqrestore(&dgcusbdcp_lock, flags);

		kfree(udwa);

		spin_lock_irqsave(&dgcusbdcp_lock, flags);
	}
	spin_unlock_irqrestore(&dgcusbdcp_lock, flags);
}

static void dgcusbdcp_disconnect(struct usb_interface *intf)
{
    POS_DEVNODE pDevNode = usb_get_intfdata(intf);
    PUSBOSHAL pUsbOsHal;
	unsigned long flags;

    dbg("%s: %p", __FUNCTION__, pDevNode);

    revert_udwa();

    if(!pDevNode)
		return;

    pUsbOsHal = pDevNode->hwDev;
    pDevNode->hwDev = NULL;

    usb_set_intfdata (intf, NULL);

    if(!pUsbOsHal)
		return;

	if(pUsbOsHal->rx_urb) {
		usb_kill_urb(pUsbOsHal->rx_urb);
		usb_free_urb(pUsbOsHal->rx_urb);
	}

	if(pUsbOsHal->hDcp) {
		DcpDestroy(pUsbOsHal->hDcp);
	}

    kfree(pUsbOsHal);

	spin_lock_irqsave(&dgcusbdcp_lock, flags);
	if(pDevNode->hwInstNum != -1) {
		dgcusbdcp_instance_bitmap &= ~(1 << pDevNode->hwInstNum);
		pDevNode->hwInstNum = -1;
	}
	spin_unlock_irqrestore(&dgcusbdcp_lock, flags);

    kfree(pDevNode);
}



static int __init dgcusbdcp_init (void)
{
    int ret;

	dbg("%s", __FUNCTION__);

	ret = OsDcpInit();
	if(ret) {
		err("%s: DCP init failed (%d)", __FUNCTION__, ret);
		return ret;
	}

	ret = usb_register(&dgcusbdcp_driver);
	if(ret) {
		err("%s: usb_register failed (%d)", __FUNCTION__, ret);
		OsDcpExit();
		return ret;
	}


	return ret;
}


static void __exit dgcusbdcp_cleanup (void)
{
	dbg("%s", __FUNCTION__);

	revert_udwa();

	usb_deregister(&dgcusbdcp_driver);

	OsDcpExit();
}

module_init(dgcusbdcp_init);
module_exit(dgcusbdcp_cleanup);

