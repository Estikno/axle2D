use thiserror::Error;

#[derive(Debug, Error)]
pub enum CustomErrors {
    #[error("Attempting to add component to an entity without calling create component first")]
    CreateComponentNeverCalled,
    #[error("Attempting to reference a component that wasn't registered")]
    ComponentNotRegistered,
    #[error("Attempting to reference an entity that doesn't exist")]
    EntityDoesNotExist,
    #[error("Attempting to get component's data that doesn't exist")]
    ComponentDataDoesNotExist,
    
    #[error("Attempting to downcast to the wrong type")]
    DowncastToWrongType,

    #[error("Attempting to add a component to a system without calling the syste creation function first")]
    CreateSystemNeverCalled,
    #[error("Attempting to reference a system that doesn't exist")]
    SystemDoesNotExist,
    #[error("Attempting to reference a component in a system that doesn't exist")]
    ComponentInSystemDoesNotExist,
}