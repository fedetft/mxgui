
/***************************************************************************
 *   Copyright (C) 2014 by Terraneo Federico and Roberto Morlacchi and     *
 *   Domenico Frasca'                                                      *
 *   Copyright (C) 2024 by Daniele Cattaneo                                *
 *   Copyright (C) 2024 by Ignazio Neto dell'Acqua                         *
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

#if defined(_BOARD_STM32F415VG_ST25DVDISCOVERY) && defined(MXGUI_LEVEL_2)

#include "event_st25dvdiscovery.h"
#include "miosix.h"
#include "kernel/scheduler/scheduler.h"
#include "util/software_i2c.h"
#include <algorithm>

using namespace std;
using namespace miosix;

static Semaphore touchIntSema;

/**
 * Touchscreen interrupt
 */
void __attribute__((naked)) EXTI9_5_IRQHandler()
{
    saveContext();
    asm volatile("bl EXTI9_5_HandlerImpl");
    restoreContext();
}

/**
 * Touchscreen interrupt actual implementation
 */
extern "C" void __attribute__((used)) EXTI9_5_HandlerImpl()
{
    EXTI->PR = EXTI_PR_PR5;
    touchIntSema.IRQsignal();
}

namespace mxgui {

typedef Gpio<GPIOC_BASE,14> buttonKey;
typedef Gpio<GPIOE_BASE,8> joySel;
typedef Gpio<GPIOE_BASE,9> joyLeft;
typedef Gpio<GPIOE_BASE,10> joyRight;
typedef Gpio<GPIOE_BASE,11> joyUp;
typedef Gpio<GPIOE_BASE,12> joyDown;
typedef Gpio<GPIOB_BASE, 6> scl;
typedef Gpio<GPIOB_BASE, 7> sda;
typedef Gpio<GPIOI_BASE, 5> interrupt;

typedef SoftwareI2C<sda, scl> ioExtI2C;

/**
 * The registers of the stmpe811 touchscreen controller
 */
enum stmpe811regs
{   
    SYS_CTRL1=0x03,
    SYS_CTRL2=0x04,
    INT_CTRL=0x09,
    GPIO_SET_PIN=0x10,
    GPIO_CLR_PIN=0x11,
    GPIO_MP_STA=0x12,
    GPIO_DIR=0x13,
    GPIO_ALT_FUNC=0x17,
    INT_EN=0x0a,
    INT_STA=0x0B,
    TSC_CTRL=0x40,
    TSC_CFG=0x41,
    FIFO_TH=0x4a,
    FIFO_STA=0x4b,
    TSC_DATA_XYZ=0xd7,
    FIFO_SIZE=0x4C
};

template <class I2C, int Addr>
class STMPE811
{
public:
    /**
     * Write into a register in the stmpe811
     * \param reg register number
     * \param val value to be written in the register
     */
    void writeReg(unsigned char reg, unsigned char val)
    {
        I2C::sendStart();
        I2C::send(Addr);
        I2C::send(reg);
        I2C::send(val);
        I2C::sendStop();
    }

    /**
     * Read from a register of the stmpe811
     * \param reg register number
     * \param n number of bytes to read from register
     * \param pointer to a memory area of at least n bytes where the read data will
     * be stored
     */
    void readReg(unsigned char reg, int n, unsigned char *result)
    {
        if (n <= 0) return;
        I2C::sendStart();
        I2C::send(Addr);
        I2C::send(reg);
        I2C::sendStop();
        I2C::sendStart();
        I2C::send(Addr | 1);
        for (int i = 0; i < n - 1; i++) result[i] = I2C::recvWithAck();
        result[n - 1] = I2C::recvWithNack();
        I2C::sendStop();
    }

    /**
     * Perform initial configuration of the chip.
     */
    void init(void)
    {
        // To let the I2C voltages settle
        Thread::sleep(5);

        writeReg(SYS_CTRL1, 0x02); // SOFT_RESET=1
        Thread::sleep(10);
        writeReg(SYS_CTRL1, 0x00); // SOFT_RESET=0
        Thread::sleep(2);
        writeReg(SYS_CTRL2, 0x08); // !GPIO_OFF !TSC_OFF !ADC_OFF
    }

    /**
     * Clear the stmpe811 fifo
     */
    void touchFifoClear()
    {
        writeReg(FIFO_STA, 0x01); // RESET FIFO
        writeReg(FIFO_STA, 0x00); // RESET FIFO
    }

    /**
     * Configure the chip as a resistive touchscreen controller.
     */
 
    void initTouch()
    {
        //Total time to read the touchscreen is
        //TDD*2+SETTLING*3+AVE*17.2us*3= ~ 17.5ms
        writeReg(TSC_CFG,0xe4); //TSC_CFG= AVE=8, TDD=1ms, SETTLING=5ms
        writeReg(FIFO_TH,0x01); //FIFO_TH= 1
        touchFifoClear();
        
        //This may allow the chip to go out of hibernate once touch detected
        writeReg(INT_CTRL,0x01);
        writeReg(INT_EN,0x03);

        // TSC_CTRL values:
        // 1     bit 0     enabled: yes
        // 001   bit 1:3   TSC mode: X, Y only
        // 011   bit 4:6   tracking index (minimum ADC delta for triggering move): 16
        // 0     bit 7     TSC status (read only)
        writeReg(TSC_CTRL,0b0'011'001'1);
        writeReg(FIFO_TH,0x01);
    }


    /**
     * \return the touch point or (-1,-1) if no touch is in progress
     */
    Point getTouchData()
    {
        unsigned char ctrl;
        // Check if touch detected by polling the CTRL register
        readReg(TSC_CTRL, 1, &ctrl);
        if ((ctrl & 0x80) == 0)
        {
            // No touch
            lastTouchPoint = Point(-1, -1);
            return lastTouchPoint;
        }else
        {
            // Touch detected, check if there are samples in FIFO.
            // Even if a touch is detected, the FIFO may be empty if:
            // - the first/next sample is not yet ready (for example because of
            //   settling time)
            // - the pen is standing still and window tracking has discarded
            //   some samples
            // In this case, reading from TSC_DATA_XYZ will return all zeros.
            // To avoid returning incorrect event coordinates we check the FIFO
            // level, and if it is zero we simply return the last point again.
            unsigned char fifoFillLevel;
            readReg(FIFO_SIZE, 1, &fifoFillLevel);
            if (fifoFillLevel == 0) return lastTouchPoint;

            // Read the new sample
            unsigned char tsData[3];
            readReg(TSC_DATA_XYZ, 3, tsData);
            touchFifoClear();
            int x = static_cast<int>(tsData[0]) << 4 | tsData[1] >> 4;
            int y = ((static_cast<int>(tsData[1]) & 0xf) << 8) | tsData[2];
            y = 4095-y; // Y is swapped

            // Apply calibration. Values may vary from unit to unit
            const int xMin = 220;
            const int xMax = 3900;
            const int yMin = 160;
            const int yMax = 3900;
            x = (x - xMin) * 240 / (xMax - xMin);
            y = (y - yMin) * 320 / (yMax - yMin);
            x=min(239,max(0,x));
            y=min(319,max(0,y));
            
            #if defined(MXGUI_ORIENTATION_VERTICAL)
            lastTouchPoint=Point(x,y);
            #elif defined(MXGUI_ORIENTATION_HORIZONTAL)
            lastTouchPoint=Point(319-y,x);
            #elif defined(MXGUI_ORIENTATION_VERTICAL_MIRRORED)
            lastTouchPoint=Point(239-x,319-y);
            #elif defined(MXGUI_ORIENTATION_HORIZONTAL_MIRRORED)
            lastTouchPoint=Point(y,239-x);
            #else
            #error unknown orientation
            #endif

            return lastTouchPoint;
        }
    }

    /**
     * Get GPIO pin state
     * @returns Bitmask with the state of each pin (1 for high, 0 for low)
     */
    unsigned char getGPIOState()
    {
        unsigned char res;
        readReg(GPIO_MP_STA, 1, &res);
        return res;
    }

private:
    Point lastTouchPoint = Point(-1,-1);
};

static STMPE811<ioExtI2C, 0x82> touchCtrl;

static Queue<Event,10> eventQueue;
static std::function<void ()> eventCallback;

static void callback(Event e)
{
    {
        FastInterruptDisableLock dLock;
        if(eventQueue.IRQput(e)==false) return;
    }
    if(eventCallback) eventCallback();
}

template <EventType::E Type>
class ButtonState
{
public:
    void update(bool newState)
    {
        if(newState)
        {
            if(lastState==false) callback(Event(Type,EventDirection::DOWN));
            lastState=true;
        } else {
            if(lastState==true) callback(Event(Type,EventDirection::UP));
            lastState=false;
        }
    }

private:
    bool lastState=false;
};

static void waitForTouchOrButton()
{
    long long t = miosix::getTime();
    // Wait until the touchscreen interrupt fires or 20ms
    if (!touchIntSema.reset()) touchIntSema.timedWait(t+20000000LL);
    touchCtrl.writeReg(INT_STA,0x03);
}

static void eventThread(void *)
{
    ButtonState<EventType::ButtonA> aButton;
    ButtonState<EventType::ButtonJoy> joyButton;
    ButtonState<EventType::ButtonUp> upButton;
    ButtonState<EventType::ButtonDown> downButton;
    ButtonState<EventType::ButtonLeft> leftButton;
    ButtonState<EventType::ButtonRight> rightButton;
    ButtonState<EventType::ButtonADown> aButtonDown;
    ButtonState<EventType::ButtonJoyDown> joyButtonDown;
    ButtonState<EventType::ButtonUpDown> upButtonDown;
    ButtonState<EventType::ButtonDownDown> downButtonDown;
    ButtonState<EventType::ButtonLeftDown> leftButtonDown;
    ButtonState<EventType::ButtonRightDown> rightButtonDown;
    bool tPrev=false, oldButtonKey = false, oldjoySel = false, oldjoyDown = false, oldjoyLeft = false, oldjoyRight = false, oldjoyUp = false ;
    Point pOld; 
    for(;;)
    {
        waitForTouchOrButton();
        aButton.update(!buttonKey::value());
        joyButton.update(!joySel::value());
        downButton.update(!joyDown::value());
        leftButton.update(!joyLeft::value());
        rightButton.update(!joyRight::value());
        upButton.update(!joyUp::value());
        
        aButtonDown.update(!buttonKey::value() && buttonKey::value()!= oldButtonKey);
        joyButtonDown.update(!joySel::value() && joySel::value()!= oldjoySel);
        downButtonDown.update(!joyDown::value() && joyDown::value()!= oldjoyDown);
        leftButtonDown.update(!joyLeft::value() && joyLeft::value()!= oldjoyLeft);
        rightButtonDown.update(!joyRight::value() && joyRight::value()!= oldjoyRight);
        upButtonDown.update(!joyUp::value() && joyUp::value()!= oldjoyUp);

        oldButtonKey = buttonKey::value();
        oldjoySel = joySel::value();
        oldjoyDown = joyDown::value();
        oldjoyLeft = joyLeft::value();
        oldjoyRight = joyRight::value();
        oldjoyUp = joyUp::value();

        //Check touchscreen
        Point p=touchCtrl.getTouchData();
        if(p.x()>=0) //Is someone touching the screen?
        {
            //Ok, someone is touching the screen
            //Did the touch point differ that much from the previous?
            if(abs(pOld.x()-p.x())>0 || abs(pOld.y()-p.y())>0 || !tPrev)
            {
                pOld=p;
                if(tPrev==false)
                    callback(Event(EventType::TouchDown,pOld,EventDirection::DOWN));
                else callback(Event(EventType::TouchMove,pOld,EventDirection::DOWN));
            }  
            tPrev=true;
        } else {
            //No, no one is touching the screen
            if(tPrev==true)
            {
                touchCtrl.touchFifoClear();
                callback(Event(EventType::TouchUp,pOld,EventDirection::UP));
            }
            tPrev=false;
        }
    }
}

//
// class InputHandlerImpl
//

InputHandlerImpl::InputHandlerImpl()
{
    {
        FastInterruptDisableLock dLock;
        buttonKey::mode(Mode::INPUT);
        interrupt::mode(Mode::INPUT);
        joySel::mode(Mode::INPUT);
        joyDown::mode(Mode::INPUT);
        joyLeft::mode(Mode::INPUT);
        joyRight::mode(Mode::INPUT);
        ioExtI2C::init();
    }

    // Init the touchscreen controller
    touchCtrl.init();
    touchCtrl.initTouch();
   
    // Turn on SYSCFG peripheral
    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;
    RCC_SYNC();

    // Configure touchscreen interrupt
    SYSCFG->EXTICR[1] = (SYSCFG->EXTICR[1] & ~SYSCFG_EXTICR2_EXTI5_Msk) | (8 << SYSCFG_EXTICR2_EXTI5_Pos);
    EXTI->IMR |= EXTI_IMR_MR5;
    EXTI->FTSR |= EXTI_FTSR_TR5;
    NVIC_EnableIRQ(EXTI9_5_IRQn);
    NVIC_SetPriority(EXTI9_5_IRQn,15); //Low priority

    //Note that this class is instantiated only once. Otherwise
    //we'd have to think a way to avoid creating multiple threads
    Thread::create(eventThread,STACK_MIN);
}

Event InputHandlerImpl::getEvent()
{
    Event result;
    eventQueue.get(result);
    return result;
}

Event InputHandlerImpl::popEvent()
{
    FastInterruptDisableLock dLock;
    Event result;
    if(eventQueue.isEmpty() == false) {
        eventQueue.IRQget(result);
    } else {
        result = Event(EventType::None);
    }
    return result;
}

function<void ()> InputHandlerImpl::registerEventCallback(function<void ()> cb)
{
    swap(eventCallback,cb);
    return cb;
}

} //namespace mxgui

#endif // _BOARD_STM32F415VG_ST25DVDISCOVERY
