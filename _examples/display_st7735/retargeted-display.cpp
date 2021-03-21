
#include "miosix.h"
#include "mxgui/drivers/display_st7735.h"

#ifndef _BOARD_STM32F4DISCOVERY
#warning "This SPI driver has only been tested on an STM32F4DISCOVERY"
#endif //_BOARD_STM32F4DISCOVERY

using namespace miosix;
using namespace mxgui;

//Hardware mapping
using scl  = Gpio<GPIOB_BASE, 13>; //PB13,  SPI1_SCK (af5)
using sda  = Gpio<GPIOB_BASE, 15>; //PB15,  SPI1_MOSI (af5)
using csx  = Gpio<GPIOB_BASE, 4>;  //PB4,   free I/O pin
using dcx  = Gpio<GPIOA_BASE, 8>;  //PA8,   free I/O pin, used only in 4-line SPI
using resx = Gpio<GPIOC_BASE, 6>;  //PC6,   free I/O pin

/**
 * Non-abstract class retargeting DisplayGenericST7735 to the correct
 * GPIOs and SPI peripheral
 */
class MyDisplay : public DisplayGenericST7735
{
public:
    static MyDisplay& instance()
    {
        static MyDisplay singleton;
        return singleton;
    }
    
private:
    MyDisplay() : DisplayGenericST7735(csx::getPin(),dcx::getPin(),resx::getPin())
    {
        {
            FastInterruptDisableLock dLock;

            RCC->APB1ENR |= RCC_APB1ENR_SPI2EN;
            SPI2->CR1 = 0;
            SPI2->CR1 = SPI_CR1_SSM   //Software cs
                    | SPI_CR1_SSI     //Hardware cs internally tied high
                    | SPI_CR1_BR_0    //clock divider: 4  ->  10,5 MHz -> 95 ns
                    | SPI_CR1_MSTR    //Master mode
                    | SPI_CR1_SPE;    //SPI enabled

            scl::mode(Mode::ALTERNATE);     scl::alternateFunction(5);
            sda::mode(Mode::ALTERNATE);     sda::alternateFunction(5);
            // GPIO software controlled
            csx.mode(Mode::OUTPUT);
            dcx.mode(Mode::OUTPUT);
            resx.mode(Mode::OUTPUT);
        }
        initialize();
    }

    unsigned char writeRam(unsigned char data) override
    {
        SPI2->DR = data;
        while((SPI2->SR & SPI_SR_RXNE) == 0) ;
        return SPI2->DR; //Note: reading back SPI2->DR is necessary.
    }

    void writeReg(unsigned char reg, unsigned char data) override
    {
        Transaction t(csx);
        {
            Transaction c(dcx);
            writeRam(reg);
        }
        writeRam(data);
    }

    void writeReg(unsigned char reg, const unsigned char *data=0, int len=1) override
    {
        Transaction t(csx);
        {
            Transaction c(dcx);
            writeRam(reg);
        }
        if(data) for(int i = 0; i < len; i++) writeRam(*data++);
    }
};

/*
 * On boards which do not have a built-in display, MXGUI requires you to
 * implement the registerDisplayHook callback to tell MXGUI which display to
 * use. If you want to adapt this example for a board that already has a
 * display, you can register a secondary display in the main with the following
 * line
 * \code
 * DisplayManager::instance().registerDisplay(new DisplayLy091wg14<sda,scl,reset>);
 * \endcode
 * And then get the display with DisplayManager::instance().getDisplay(1).
 * Note that 0 is the default display, 1 would be the secondary one.
 */
namespace mxgui {
void registerDisplayHook(DisplayManager& dm)
{
    dm.registerDisplay(&MyDisplay::instance());
}
} //namespace mxgui
