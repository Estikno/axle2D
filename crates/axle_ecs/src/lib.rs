use std::any::Any;

use resource::Resource;

mod resource;

#[derive(Default)]
pub struct World {
    resources: Resource
}

impl World {
    pub fn new() -> Self {
        Self::default()
    }

    /// Add a resource to the world so that anyone with access to the world can query it.
    /// 
    /// Resources are stored based on their type id.
    /// 
    /// # Example
    /// 
    /// ```
    /// use axle_ecs::World;
    /// 
    /// let mut world = World::new();
    /// world.add_resource(10_i32);
    /// ```
    pub fn add_resource(&mut self, resource_data: impl Any) {
        self.resources.add(resource_data);
    }

    /// Query for a resource and get an immutable reference to it.
    /// 
    /// The type of the resource must be added in so that it can be found.
    /// 
    /// # Example
    /// 
    /// ```
    /// use axle_ecs::World;
    /// 
    /// let mut world = World::new();
    /// world.add_resource(10_i32);
    /// 
    /// let resource = world.get_resource::<i32>().unwrap();
    /// 
    /// assert_eq!(*resource, 10);
    /// ```
    pub fn get_resource<T: Any>(&self) -> Option<&T> {
        self.resources.get_ref::<T>()
    }

    /// Query for a resource and get a mutable reference to it.
    /// 
    /// The type of the resource must be added in so that it can be found.
    /// 
    /// # Example
    /// 
    /// ```
    /// use axle_ecs::World;
    /// 
    /// let mut world = World::new();
    /// world.add_resource(10_i32);
    /// 
    /// {
    ///     let resource = world.get_resource_mut::<i32>().unwrap();
    ///     *resource += 1;
    /// }
    /// 
    /// let resource = world.get_resource::<i32>().unwrap();
    /// 
    /// assert_eq!(*resource, 11);
    /// ```
    pub fn get_resource_mut<T: Any>(&mut self) -> Option<&mut T> {
        self.resources.get_mut::<T>()
    }

    /// This will remove the resource from the world.
    /// If the resource doesn't exist, it won't happen anything.
    /// 
    /// # Example
    /// 
    /// ```
    /// use axle_ecs::World;
    /// 
    /// let mut world = World::new();
    /// world.add_resource(10_i32);
    /// 
    /// world.delete_resource::<i32>();
    /// 
    /// assert!(world.get_resource::<i32>().is_none());
    /// ```
    pub fn delete_resource<T: Any>(&mut self) {
        self.resources.remove::<T>();
    }
}

#[cfg(test)]
mod tests {}
