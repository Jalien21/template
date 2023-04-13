#include "daisy_petal.h"
#include "daisysp.h" 
#include "terrarium.h"

using namespace daisy;
using namespace daisysp;
using namespace terrarium;


DaisyPetal hw; // Declare a local daisy_petal for hardware access
DaisySeed hw2; // This is needed for serial print, but should only need one of these, right?

dsy_gpio led1;
dsy_gpio led2;

Parameter vtime, vfreq, vsend;
float vtime_prev, vfreq_prev, vsend_prev;

bool bypass, maxverb;
ReverbSc verb;

void doSwitchThings();

// This runs at a fixed rate, to prepare audio samples
void callback(AudioHandle::InputBuffer  in,
                   AudioHandle::OutputBuffer out,
                   size_t                    size){
    float dryl, dryr, wetl, wetr, sendl, sendr;
    hw.ProcessAllControls();
    if (!maxverb) verb.SetFeedback(vtime.Process());
    verb.SetLpFreq(vfreq.Process());
    vsend.Process(); // Process Send to use later

    doSwitchThings();

    for (size_t i = 0; i < size; i++)
    {
        dryl = in[0][i];
        dryr = in[1][i];
        sendl = dryl * vsend.Value();
        sendr = dryr * vsend.Value();
        verb.Process(sendl, sendr, &wetl, &wetr);
        if (bypass)
        {
            out[0][i] = in[0][i]; // left
            out[1][i] = in[1][i]; // right
        }
        else
        {
            out[0][i] = dryl + wetl;
            out[1][i] = dryr + wetr;
        }
    }
}

int main(void)
{
    float samplerate;

    hw.Init();
    samplerate = hw.AudioSampleRate();

    hw2.StartLog(); // enable logging and set up USB

    vtime.Init(hw.knob[Terrarium::KNOB_1], 0.6f, 0.999f, Parameter::LOGARITHMIC);
    vfreq.Init(hw.knob[Terrarium::KNOB_2], 500.0f, 20000.0f, Parameter::LOGARITHMIC);
    vsend.Init(hw.knob[Terrarium::KNOB_3], 0.0f, 1.0f, Parameter::LINEAR);
    verb.Init(samplerate);

    vtime_prev = 0;

    hw.StartAdc();
    hw.StartAudio(callback);

    led1.pin = hw.seed.GetPin(Terrarium::LED_1);
    led1.mode = DSY_GPIO_MODE_OUTPUT_PP;
    led1.pull = DSY_GPIO_NOPULL;
    dsy_gpio_init(&led1);

    led2.pin = hw.seed.GetPin(Terrarium::LED_2);
    led2.mode = DSY_GPIO_MODE_OUTPUT_PP;
    led2.pull = DSY_GPIO_NOPULL;
    dsy_gpio_init(&led2);

    bypass = false;
	maxverb = false;

    hw2.PrintLine("Init Complete");

    while(1) 
    {
        // Do Stuff InfInitely Here
        hw.DelayMs(10);
        dsy_gpio_write(&led1, bypass ? 0 : 1);

        if(abs(vtime.Value()-vtime_prev)>0.0001) hw2.PrintLine("vtime = " FLT_FMT3, FLT_VAR3(vtime.Value()));
        if(abs(vfreq.Value()-vfreq_prev)>10) hw2.PrintLine("vfreq = " FLT_FMT3, FLT_VAR3(vfreq.Value()));
        if(abs(vsend.Value()-vsend_prev)>0.001) hw2.PrintLine("vsend = " FLT_FMT3, FLT_VAR3(vsend.Value()));

        vtime_prev = vtime.Value();
        vfreq_prev = vfreq.Value();
        vsend_prev = vsend.Value();
    }
}

void doSwitchThings()
{
    if (hw.switches[Terrarium::FOOTSWITCH_1].RisingEdge())
        bypass = !bypass;
    if (hw.switches[Terrarium::FOOTSWITCH_2].RisingEdge() && !bypass && !maxverb)
    {
        maxverb = true;
        dsy_gpio_write(&led2, 1);
        verb.SetFeedback(0.999f);
        hw2.PrintLine("Max Verb On");
    }
    else if (hw.switches[Terrarium::FOOTSWITCH_2].FallingEdge())
    {
        maxverb = false;
        dsy_gpio_write(&led2, 0);
        hw2.PrintLine("Max Verb Off");
    }
    if (hw.switches[Terrarium::SWITCH_1].RisingEdge()) hw2.PrintLine("SW1 on");
    if (hw.switches[Terrarium::SWITCH_2].RisingEdge()) hw2.PrintLine("SW2 on");
    if (hw.switches[Terrarium::SWITCH_3].RisingEdge()) hw2.PrintLine("SW3 on");
    if (hw.switches[Terrarium::SWITCH_4].RisingEdge()) hw2.PrintLine("SW4 on");
    if (hw.switches[Terrarium::SWITCH_1].FallingEdge()) hw2.PrintLine("SW1 off");
    if (hw.switches[Terrarium::SWITCH_2].FallingEdge()) hw2.PrintLine("SW2 off");
    if (hw.switches[Terrarium::SWITCH_3].FallingEdge()) hw2.PrintLine("SW3 off");
    if (hw.switches[Terrarium::SWITCH_4].FallingEdge()) hw2.PrintLine("SW4 off");

    //if(vtime_prev != vtime.Value()) hw2.PrintLine("vtime = " FLT_FMT3, FLT_VAR3(vtime.Value()));
    //if(vfreq_prev != vfreq.Value()) hw2.PrintLine("vfreq = " FLT_FMT3, FLT_VAR3(vfreq.Value()));
    //if(vsend_prev != vsend.Value()) hw2.PrintLine("vsend = " FLT_FMT3, FLT_VAR3(vsend.Value()));


}