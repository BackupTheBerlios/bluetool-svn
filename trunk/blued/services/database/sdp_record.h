#ifndef __SDP_RECORD_H
#define __SDP_RECORD_H

#include "../../libbluetooth/sdpsession.h"
#include "../../libbluetooth/sdprecord.h"

/* basically, when the user is asked to start/stop a service,
   he makes a request to the sdp.manager interface using
   the methods StartService, StopService giving the service name
   as the parameter, the manager uses DBUS service activation to start
   the service, or calls Stop() on the sdp.service interface to stop it.
   When a service is started it SHOULD have created a subclass of SdpRecord
   (the class declared in this source file) and when the service is
   started, should call register() on this object (this happens after
   the record has been populated with the necessary information, of course)
   register() takes the dbus path name as a parameter (stg like /org/bluetool/profiles/0)
   Every profile is identified by its record handle
   The SDP server has an associated profile as well (so you start/stop the
   server as you do with any other bluetooth service)
   but it has some particularities:
    every other service depends upon it
    it ALWAYS has the service handle value equal to 0
*/

class SdpRecord

#endif//__SDP_RECORD_H
