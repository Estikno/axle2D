use thiserror::Error;

#[derive(Debug, Error)]
pub enum CustomErrors {
    #[error("Attempting to access add component to an entity without calling create component first")]
    CreateComponentNeverCalled,
    #[error("Attempted to insert data for component that wasn't registered")]
    ComponentNotRegistered
}