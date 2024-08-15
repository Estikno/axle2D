use std::rc::Rc;

use axle2D::application::App;
use axle2D::config::{GlobalConfig, Resolution};

use axle2D::engine::Vector2;

use axle2D::engine::Keycode;
use axle2D::engine::MouseButton;

//mods
mod object_example;

fn main() {
    let global_config = GlobalConfig::new(Resolution { width: 1024, height: 576}, String::from("Axle Tests"), 4_f32, 60);
    let rc_global_config = Rc::new(global_config);

    let mut app = App::new(rc_global_config);

    let mut new_object = object_example::Rectangle::new(
        Vector2::new(100_f32, 100_f32),
        0_f32,
        Vector2::one(),
        true
    );
    //new_object.set_rigidbody();

    let objects = vec![object_example::Rectangle::new(
        Vector2::new(50_f32, 100_f32),
        0_f32,
        Vector2::one(),
        false
    ),
    object_example::Rectangle::new(
        Vector2::new(100_f32, 500_f32),
        0_f32,
        Vector2::one(),
        false
    ),
    object_example::Rectangle::new(
        Vector2::new(500_f32, 400_f32),
        0_f32,
        Vector2::one(),
        false
    ),
    object_example::Rectangle::new(
        Vector2::new(300_f32, 200_f32),
        0_f32,
        Vector2::one(),
        false
    ),
    object_example::Rectangle::new(
        Vector2::new(281_f32, 90_f32),
        0_f32,
        Vector2::one(),
        false
    ),
    object_example::Rectangle::new(
        Vector2::new(900_f32, 32_f32),
        0_f32,
        Vector2::one(),
        false
    ),
    object_example::Rectangle::new(
        Vector2::new(42_f32, 45_f32),
        0_f32,
        Vector2::one(),
        false
    ),
    object_example::Rectangle::new(
        Vector2::new(423_f32, 185_f32),
        0_f32,
        Vector2::one(),
        false
    )];

    app.add_object(Box::new(new_object));

    for mut object in objects.into_iter() {
        //object.set_rigidbody();
        app.add_object(Box::new(object));
    }
        
    loop {
        let (exit, frame_start) = app.update();

        if exit { break }

        app.wait(frame_start);
    }
}

