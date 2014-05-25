/***************************************************************************
 *   Copyright (C) 2011 by Terraneo Federico                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   As a special exception, if other files instantiate templates or use   *
 *   macros or inline functions from this file, or you compile this file   *
 *   and link it with other works to produce a work based on this file,    *
 *   this file does not by itself cause the resulting work to be covered   *
 *   by the GNU General Public License. However the source code for this   *
 *   file must still be made available in accordance with the GNU General  *
 *   Public License. This exception does not invalidate any other reasons  *
 *   why a work based on this file might be covered by the GNU General     *
 *   Public License.                                                       *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, see <http://www.gnu.org/licenses/>   *
 ***************************************************************************/

#include <config/mxgui_settings.h>
#include "display.h"

#ifdef MXGUI_LEVEL_2

#ifndef APPLICATION_H
#define	APPLICATION_H

namespace mxgui {

class ApplicationImpl; //Forward declarartion

/**
 * \ingroup pub_iface_2
 * Applications are the central point of mxgui level 2, this part of the library
 * allows multiple applications to run concurrently and share the same display.
 * Applications are classes that derive from Application and override the
 * run() member function.
 */
class Application
{
public:
    /**
     * Constructor
     */
    Application();

    virtual ~Application();

protected:

    /**
     * Applications that do not override this member function will get the
     * default event loop that will call the callbacks registered during the
     * Application's constructor.
     * Appications that override this member function should implement a message
     * loop themselves. Also, within tht message loop unused events should be
     * passed to dispatchEvent(), or event callbacks won't be fired.
     */
    virtual void run();

    Display& getDisplay();

    Event getEvent();

    Event popEvent();

    void dispatchEvent(Event e);
    
private:
    ApplicationImpl *pImpl;
};

/**
 * \ingroup pub_iface_2
 * The application manager is a class used mainly to start applications
 */
class ApplicationManager
{
public:

    /**
     * \param app Application to start. The pointer must point to an heap
     * allocated instance of an application class. The pointer is deleted
     * internally by the ApplicationManager when the application terminates.
     * Since the pointer passed to start() can be deleted at any time, it is
     * undefined behaviour to dereference it after the call to start().
     * The suggested way to call this member function is
     * \code
     * class MyApp : public application
     * [...]
     * ApplicationManager::start(new MyApp);
     * \endcode
     * \param modal if true the application will be started in the thread that
     * calls start, and therefore the call to start won't return till the
     * application will terminate.
     * \return true if success, false on failure
     */
    static bool start(Application *app, bool modal=false);
};

} //namespace mxgui

#endif //APPLICATION_H

#endif //MXGUI_LEVEL_2
