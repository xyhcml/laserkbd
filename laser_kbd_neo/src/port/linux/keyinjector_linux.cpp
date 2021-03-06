/*                                                                              
 * Copyright (C) 2013 Deepin, Inc.                                                 
 *               2013 Leslie Zhai                                                  
 *                                                                              
 * Author:     Leslie Zhai <zhaixiang@linuxdeepin.com>                           
 * Maintainer: Leslie Zhai <zhaixiang@linuxdeepin.com>                           
 *                                                                              
 * This program is free software: you can redistribute it and/or modify         
 * it under the terms of the GNU General Public License as published by         
 * the Free Software Foundation, either version 3 of the License, or            
 * any later version.                                                           
 *                                                                              
 * This program is distributed in the hope that it will be useful,              
 * but WITHOUT ANY WARRANTY; without even the implied warranty of               
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                
 * GNU General Public License for more details.                                 
 *                                                                              
 * You should have received a copy of the GNU General Public License            
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.        
 */

#include <iostream>
#include <atspi/atspi.h>
#include <X11/Xlib.h>

#include "common.h"
#include "port/common/keyinjector.h"
#include "config_mgr.h"

extern ConfigBundle         g_config_bundle;

class OSKeyInjector_Linux : public OSKeyInjector
{
public :
    OSKeyInjector_Linux() :m_display(NULL) 
    {
        m_display = XOpenDisplay(NULL);
        if (m_display == NULL) 
            std::cout << "ERROR: Could not open display" << std::endl;
    }

    virtual ~OSKeyInjector_Linux() 
    {
        if (m_display) 
        {
            XCloseDisplay(m_display);
            m_display = NULL;
        }
    }

    virtual bool injectKeyEvents(const std::vector<KeyEventDesc> & intputlist)
    {
        bool hasinputs = false;
        int keyval;
        int keycode;
        int pos;
        
        if (!intputlist.size()) 
            return false;                                   
                                                                                
        for (pos = 0; pos < intputlist.size(); ++pos) 
        {                                                                   
            keyval = intputlist[pos].keyval;
            keycode = m_KeysymToKeycode(keyval);
            //printf("DEBUG: keyval %d keycode %d\n", keyval, keycode);

            if (intputlist[pos].type == KEY_EVENT_PRESSED) 
            {                
                hasinputs = true;
                atspi_generate_keyboard_event(keycode, NULL, ATSPI_KEY_PRESS, NULL);
            } 
            else 
                atspi_generate_keyboard_event(keycode, NULL, ATSPI_KEY_RELEASE, NULL);         
        }                    
                                                                                
        if (hasinputs && g_config_bundle.playsound) 
        {                           
            // play sound feedback                                              
            std::string soundfile = FILEPATH_RESOURCE_SOUND_FOLDER;             
            soundfile += "type.wav";                                            
        }                                                                       
                                                                                
        return true;
    }

private:
    int m_KeysymToKeycode(int Keysym) 
    {
        if (m_display) 
            return XKeysymToKeycode(m_display, Keysym);

        return -1;
    }

private:
    Display* m_display;
};

OSKeyInjector* OSKeyInjector::g_inst = NULL;
static pthread_mutex_t m_mutex = PTHREAD_MUTEX_INITIALIZER;

OSKeyInjector * OSKeyInjector::GetInstance() {
    pthread_mutex_lock(&m_mutex);
    if (OSKeyInjector::g_inst) 
    {
        pthread_mutex_unlock(&m_mutex);
        return OSKeyInjector::g_inst;
    }

    OSKeyInjector::g_inst = new OSKeyInjector_Linux();
    pthread_mutex_unlock(&m_mutex);
    return OSKeyInjector::g_inst;
}

void OSKeyInjector::ReleaseInstance()
{
    pthread_mutex_lock(&m_mutex);
    if (OSKeyInjector::g_inst) 
    {
        delete OSKeyInjector::g_inst;
        OSKeyInjector::g_inst = NULL;
    }
    pthread_mutex_unlock(&m_mutex);
}
