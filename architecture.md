# Event system

explo's architecture is event-based. In order to permit an efficient decoupling of its components, the communication
is achieved through events. We distinguish two types of events: **notifications** and **actions**.

### Notification

Notifications are events that generally are dispatched without any specified recipient. An exmaple is the
`entity_move_notification`: as soon as an entity moves, a notification is dispatched which can be processed by multiple
handlers that can perform
different actions.

### Action

An action event is generally dispatch to make a specific component achieve a certain action. An example is the
`chunk_load_action`, which is dispatched to make the world load a specific chunk.

A **system** is a class that has one or more `event_handler(s)` and is responsible for listening for events (
notifications
or actions) and output other events (again, notifications or actions, but mostly actions).

###### Multithreading note

Systems are supposed to outlive the event system. In other words, it's safe to reference to them in asynchronous event
jobs, as they will always be valid.

