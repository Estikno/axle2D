use std::rc::Rc;

use axle::application::App;

use axle::engine::Vector2;

use axle::engine::Keycode;
use axle::engine::MouseButton;

fn main() {
    let global_config = GlobalConfig::new(
        Resolution {
            width: 1024,
            height: 576,
        },
        String::from("Axle Tests"),
        4_f32,
        60,
    );
    let rc_global_config = Rc::new(global_config);

    let mut app = App::new(rc_global_config);

    let mut new_object =
        object_example::Rectangle::new(Vector2::new(100_f32, 100_f32), 0_f32, Vector2::one(), true);
    new_object.set_rigidbody();

    let objects = vec![
        object_example::Rectangle::new(Vector2::new(50_f32, 50_f32), 0_f32, Vector2::one(), false),
        object_example::Rectangle::new(Vector2::new(100_f32, 50_f32), 0_f32, Vector2::one(), false),
        object_example::Rectangle::new(Vector2::new(150_f32, 50_f32), 0_f32, Vector2::one(), false),
        object_example::Rectangle::new(Vector2::new(200_f32, 50_f32), 0_f32, Vector2::one(), false),
        object_example::Rectangle::new(Vector2::new(250_f32, 50_f32), 0_f32, Vector2::one(), false),
        object_example::Rectangle::new(Vector2::new(300_f32, 50_f32), 0_f32, Vector2::one(), false),
        object_example::Rectangle::new(Vector2::new(350_f32, 50_f32), 0_f32, Vector2::one(), false),
        object_example::Rectangle::new(Vector2::new(400_f32, 50_f32), 0_f32, Vector2::one(), false),
    ];

    app.add_object(Box::new(new_object));

    for mut object in objects.into_iter() {
        object.set_rigidbody();
        app.add_object(Box::new(object));
    }

    loop {
        let (exit, frame_start) = app.update();

        if exit {
            break;
        }

        app.wait(frame_start);
    }
}
