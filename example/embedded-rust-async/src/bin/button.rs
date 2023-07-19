#![no_std]
#![no_main]
#![feature(type_alias_impl_trait)]

use embassy_executor::Spawner;
use embassy_stm32::gpio::{Level, Input, Output, Pull, Speed};
use embassy_stm32::exti::ExtiInput;
use embassy_stm32::peripherals::{PC13, PH7};
use embassy_time::{Duration, Ticker};
use {defmt_rtt as _, panic_probe as _};

use core::sync::atomic::{AtomicBool, Ordering};

static LED_BLINK_ACTIVE: AtomicBool = AtomicBool::new(true);

#[embassy_executor::main]
async fn main(spawner: Spawner) {
    let p = embassy_stm32::init(Default::default());

    let led = Output::new(p.PH7, Level::High, Speed::Medium);
    let button = Input::new(p.PC13, Pull::Down);
    let button = ExtiInput::new(button, p.EXTI13);

    spawner.spawn(buttony(button)).unwrap();
    spawner.spawn(blinky(led)).unwrap();
}

#[embassy_executor::task]
async fn blinky(mut led: Output<'static, PH7>) -> ! {
    const LED_PERIOD_MS: u64 = 200;

    let mut ticker = Ticker::every(Duration::from_millis(LED_PERIOD_MS));
    loop {
        ticker.next().await;
        if LED_BLINK_ACTIVE.load(Ordering::Relaxed) {
            led.toggle();
        }
    }
}

#[embassy_executor::task]
async fn buttony(mut button: ExtiInput<'static, PC13>) {
    loop {
        button.wait_for_rising_edge().await;

        LED_BLINK_ACTIVE.fetch_xor(true, Ordering::SeqCst);
    }
}
