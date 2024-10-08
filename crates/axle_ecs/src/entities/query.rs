use std::any::{Any, TypeId};
use eyre::Result;

use crate::prelude::*;
use super::Entities;

pub type QueryIndexes = Vec<usize>;
pub type QueryComponents = Vec<Vec<Component>>;

/// A query that can be used to query the entities in the world and its components in an easy way.
#[derive(Debug)]
pub struct Query<'a> {
    /// The bit mask of the query. This is used to filter out entities that don't have all the
    /// components that are required in the query.
    map: u32,
    /// The entities that the query is run on.
    entities: &'a Entities,
    /// A vector of type ids of the components that are required in the query.
    type_ids: Vec<TypeId>,
}

impl<'a> Query<'a> {
    /// Creates a new `Query` struct.
    ///
    /// # Arguments
    ///
    /// * `entities` - The `Entities` struct to query.
    ///
    /// # Returns
    ///
    /// A new `Query` struct.
    pub fn new(entities: &'a Entities) -> Self {
        Self { entities, map: 0, type_ids: vec![] }
    }

    /// Adds a component to the query.
    ///
    /// # Arguments
    ///
    /// * `T` - The type of the component to add.
    ///
    /// # Returns
    ///
    /// A result that contains a mutable reference to the `Query` struct if succeeds or an error if it fails.
    ///
    /// # Errors
    ///
    /// If the component was not registered, an error is returned.
    pub fn with_component<T: Any>(&mut self) -> Result<&mut Self> {
        let type_id = TypeId::of::<T>();
        if let Some(bit_mask) = self.entities.get_bitmask(&type_id) {
            self.map |= bit_mask;
            self.type_ids.push(type_id);
        }
        else {
            return Err(CustomErrors::ComponentNotRegistered.into());
        }

        Ok(self)
    }

    /// Adds a component to the query by its type id.
    ///
    /// # Arguments
    ///
    /// * `type_id` - The type id of the component to add.
    ///
    /// # Returns
    ///
    /// A result that contains a mutable reference to the `Query` struct if succeeds or an error if it fails.
    ///
    /// # Errors
    ///
    /// If the component was not registered, an error is returned.
    pub fn with_component_by_type_id(&mut self, type_id: TypeId) -> Result<&mut Self> {
        if let Some(bit_mask) = self.entities.get_bitmask(&type_id) {
            self.map |= bit_mask;
            self.type_ids.push(type_id);
        }
        else {
            return Err(CustomErrors::ComponentNotRegistered.into());
        }

        Ok(self)
    }

    /// Runs the query and returns the results.
    ///
    /// # Returns
    ///
    /// A tuple containing two items:
    ///
    /// - The first item is a vector of indexes of the entities that matched the query.
    ///
    /// - The second item is a vector of vectors of components that matched the query.
    pub fn run(&self) -> (QueryIndexes, QueryComponents) {
        // Get the indexes of the entities that matched the query
        let indexes: Vec<usize> = self.entities.map
            .iter()
            .enumerate()
            .filter_map(|(index, entity_map)| {
                if entity_map & self.map == self.map {
                    Some(index)
                }
                else {
                    None
                }
            })
            .collect();

        // Get the components of the entities that matched the query
        let mut result = vec![];

        for type_id in &self.type_ids {
            let entity_components = self.entities.components.get(type_id).unwrap();
            let mut components_to_keep = vec![];

            for index in &indexes {
                components_to_keep.push(entity_components[*index].as_ref().unwrap().clone());
            }

            result.push(components_to_keep);
        }

        (indexes, result)
    }

    /// Runs the query and returns a vector of `QueryEntity`s
    ///
    /// This function is similar to `run`, but it returns a vector of `QueryEntity`s instead of a tuple of indexes and components.
    ///
    /// # Returns
    ///
    /// A vector of `QueryEntity`s
    pub fn run_entity(&self) -> Vec<QueryEntity> {
        // Iterate over the entities and their bitmasks
        self.entities.map
            .iter()
            .enumerate()
            .filter_map(|(index, entity_map)| {
                // Check if the entity's bitmask matches the query's bitmask
                if entity_map & self.map == self.map {
                    // If it matches, create a new `QueryEntity` and return it
                    Some(QueryEntity::new(index, self.entities))
                }
                else {
                    // If it doesn't match, return None
                    None
                }
            })
            .collect()
    }
}

#[cfg(test)]
mod tests {
    use std::cell::{Ref, RefMut};

    use crate::entities::query_entity::QueryEntity;
    use super::*;

    #[test]
    fn query_mask_updating_with_component() -> Result<()> {
        let mut entities = Entities::default();
        entities.register_component::<u32>();
        entities.register_component::<f32>();

        let mut query = Query::new(&entities);
        query
            .with_component::<u32>()?
            .with_component::<f32>()?;

        assert_eq!(query.map, 3);
        assert_eq!(TypeId::of::<u32>(), query.type_ids[0]);
        assert_eq!(TypeId::of::<f32>(), query.type_ids[1]);

        Ok(())
    }

    #[test]
    fn run_query() -> Result<()> {
        let mut entities = Entities::default();
        entities.register_component::<u32>();
        entities.register_component::<f32>();
        
        entities
            .create_entity()
            .with_component(10_u32)?
            .with_component(20.0_f32)?;

        entities
            .create_entity()
            .with_component(5_u32)?;

        entities
            .create_entity()
            .with_component(50.0_f32)?;

        entities
            .create_entity()
            .with_component(15_u32)?
            .with_component(25.0_f32)?;

        let mut query = Query::new(&entities);
        query
            .with_component::<u32>()?
            .with_component::<f32>()?;

        let query_result = query.run();
        let u32s = &query_result.1[0];
        let f32s = &query_result.1[1];
        let indexes = &query_result.0;

        assert!(u32s.len() == f32s.len() && u32s.len() == indexes.len());
        assert_eq!(u32s.len(), 2);

        let wrapped_first_u32 = u32s[0].borrow();
        let first_u32 = wrapped_first_u32.downcast_ref::<u32>().unwrap();
        assert_eq!(*first_u32, 10);

        let wrapped_first_f32 = f32s[0].borrow();
        let first_f32 = wrapped_first_f32.downcast_ref::<f32>().unwrap();
        assert_eq!(*first_f32, 20.0);

        let wrapped_second_u32 = u32s[1].borrow();
        let second_u32 = wrapped_second_u32.downcast_ref::<u32>().unwrap();
        assert_eq!(*second_u32, 15);

        let wrapped_second_f32 = f32s[1].borrow();
        let second_f32 = wrapped_second_f32.downcast_ref::<f32>().unwrap();
        assert_eq!(*second_f32, 25.0);

        assert_eq!(indexes[0], 0);
        assert_eq!(indexes[1], 3);

        Ok(())
    }

    #[test]
    fn query_for_entity_ref() -> Result<()> {
        let mut entitities = Entities::default();
        entitities.register_component::<u32>();
        entitities.register_component::<f32>();

        entitities
            .create_entity()
            .with_component(100_u32)?;
        entitities
            .create_entity()
            .with_component(10.0_f32)?;

        let mut query = Query::new(&entitities);
        let entitities: Vec<QueryEntity> = query
            .with_component::<u32>()?
            .run_entity();

        assert_eq!(entitities.len(), 1);

        for entity in entitities {
            assert_eq!(entity.id, 0);
            let health: Ref<u32> = entity.get_component::<u32>()?;
            assert_eq!(*health, 100);
        }

        Ok(())
    }

    #[test]
    fn query_for_entity_mut() -> Result<()> {
        let mut entitities = Entities::default();
        entitities.register_component::<u32>();
        entitities.register_component::<f32>();

        entitities
            .create_entity()
            .with_component(100_u32)?;
        entitities
            .create_entity()
            .with_component(10.0_f32)?;

        let mut query = Query::new(&entitities);
        let entitities: Vec<QueryEntity> = query
            .with_component::<u32>()?
            .run_entity();

        assert_eq!(entitities.len(), 1);

        for entity in entitities {
            assert_eq!(entity.id, 0);
            let mut health: RefMut<u32> = entity.get_component_mut::<u32>()?;
            assert_eq!(*health, 100);
            *health += 1;
        }

        let entitities: Vec<QueryEntity> = query
            .with_component::<u32>()?
            .run_entity();
        for entity in entitities {
            let health: Ref<u32> = entity.get_component::<u32>()?;
            assert_eq!(*health, 101);
        }

        Ok(())
    }
}