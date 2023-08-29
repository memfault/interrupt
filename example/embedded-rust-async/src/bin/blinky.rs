#![no_std]
#![no_main]
#![feature(type_alias_impl_trait)]

use embassy_executor::Spawner;
use embassy_stm32::gpio::{Level, Output, Speed};
use embassy_stm32::peripherals::PH7;
use embassy_time::{Duration, Ticker};
use {defmt_rtt as _, panic_probe as _};

#[embassy_executor::main]
async fn main(spawner: Spawner) {
    let p = embassy_stm32::init(Default::default());
    let led = Output::new(p.PH7, Level::Low, Speed::Medium);

    spawner.spawn(blinky(led)).unwrap();
}

#[embassy_executor::task]
async fn blinky(mut led: Output<'static, PH7>) -> ! {
    const LED_PERIOD_MS: u64 = 200;

    let mut ticker = Ticker::every(Duration::from_millis(LED_PERIOD_MS));

    loop {
        led.toggle();
        ticker.next().await;
    }
}
