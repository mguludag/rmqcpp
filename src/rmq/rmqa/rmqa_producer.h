// Copyright 2020-2023 Bloomberg Finance L.P.
// SPDX-License-Identifier: Apache-2.0
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// rmqa_producer.h
#ifndef INCLUDED_RMQA_PRODUCER
#define INCLUDED_RMQA_PRODUCER

#include <rmqa_topologyupdate.h>
#include <rmqp_producer.h>
#include <rmqp_topology.h>
#include <rmqt_exchange.h>
#include <rmqt_message.h>
#include <rmqt_queue.h>
#include <rmqt_result.h>

#include <bsl_string.h>
#include <bslma_managedptr.h>
#include <bsls_timeinterval.h>

namespace BloombergLP {
namespace rmqa {

/// \class Producer
/// \brief Provide a RabbitMQ Producer API for publishing to a specific
///        Exchange
///
/// A RabbitMQ Message Producer API object. A Producer is bound to a specific
/// Exchange. These objects are constructed by rmqa::VHost#createProducer
class Producer {
  public:
    // CREATORS
    /// Create an instance of a Producer that will operate through the supplied
    /// producer implementation. The Producer object takes ownership of the
    /// supplied producer implementation
    explicit Producer(bslma::ManagedPtr<rmqp::Producer>& impl);

    ~Producer();

    // MANIPULATORS
    /// Send a message with the given `routingKey` to the exchange this
    /// Producer targets
    ///
    /// This call returns immediately when there are fewer unconfirmed messages
    /// than the maxOutstandingConfirms limit configured when calling
    /// Connection::createProducer
    ///
    /// All messages are sent with mandatory = true, immediate = false. Other
    /// values can be sent with a different Producer::send call, but these
    /// defaults were chosen for safety. Read the warnings carefully.
    ///
    /// \param message    The message to be sent
    /// \param routingKey The routing key (e.g. topic, or queue name) passed to
    ///                   the exchange.
    /// \param confirmCallback Called when the broker explicitly
    ///                      confirms/rejects
    ///                      the message. Messages are automatically retried
    ///                      on reconnection, in which case this method may be
    ///                      called sometime after invoking `send`.
    /// \param timeout    How long to wait for as a relative timeout. If
    ///                   timeout is 0, the method will wait to send message
    ///                   indefinitely
    ///
    /// \return SENDING   When the library accepts this message for sending.
    ///                   Note `rmq` will resend the message if the connection
    ///                   drops without receiving a confirm
    /// \return DUPLICATE Returned if a `message` with the same GUID has
    ///                   already been sent and is awaiting a confirm from the
    ///                   broker. This indicates an issue with the application.
    ///                   When sending duplicate messages a new message should
    ///                   be constructed
    /// \return TIMEOUT   Returned if a message couldn't be enqueued within the
    ///                   timeout time
    rmqp::Producer::SendStatus
    send(const rmqt::Message& message,
         const bsl::string& routingKey,
         const rmqp::Producer::ConfirmationCallback& confirmCallback,
         const bsls::TimeInterval& timeout = bsls::TimeInterval());

    /// Send a message with the given `routingKey` to the exchange this
    /// Producer targets, with the specified mandatory flag.
    /// Use the simpler Producer::send() unless you understand & intend to set
    /// different values for mandatory.
    ///
    /// This call returns immediately when there are fewer unconfirmed messages
    /// than the maxOutstandingConfirms limit configured when calling
    /// Connection::createProducer
    ///
    /// \param message    The message to be sent
    /// \param routingKey The routing key (e.g. topic, or queue name) passed to
    ///                   the exchange.
    /// \param mandatory  Specify the mandatory flag:
    ///                   RETURN_UNROUTABLE (Recommended): Any messages not
    ///                   passed to a queue are returned to the sender.
    ///                   confirmCallback will be invoked with a RETURN status.
    ///                   DISCARD_UNROUTABLE (**Dangerous**): Any messages not
    ///                   passed to a queue are confirmed by the broker. This
    ///                   will cause silent message loss in the event bindings
    ///                   aren't setup as expected.
    /// \param confirmCallback Called when the broker explicitly
    ///                      confirms/rejects
    ///                      the message. Messages are automatically retried
    ///                      on reconnection, in which case this method may be
    ///                      called sometime after invoking `send`.
    /// \param timeout    How long to wait for as a relative timeout. If
    ///                   timeout is 0, the method will wait to send message
    ///                   indefinitely
    ///
    /// \return SENDING   When the library accepts this message for sending.
    ///                   Note `rmq` will resend the message if the connection
    ///                   drops without receiving a confirm
    /// \return DUPLICATE Returned if a `message` with the same GUID has
    ///                   already been sent and is awaiting a confirm from the
    ///                   broker. This indicates an issue with the application.
    ///                   When sending duplicate messages a new message should
    ///                   be constructed
    /// \return TIMEOUT   Returned if a message couldn't be enqueued within the
    ///                   timeout time
    rmqp::Producer::SendStatus
    send(const rmqt::Message& message,
         const bsl::string& routingKey,
         rmqt::Mandatory::Value mandatoryFlag,
         const rmqp::Producer::ConfirmationCallback& confirmCallback,
         const bsls::TimeInterval& timeout = bsls::TimeInterval());

    /// Updates topology and waits for the server to confirm the update status
    ///
    /// \param timeout   How long to wait for. If timeout is 0, the method will
    ///                indefinitely wait for confirms.
    ///
    /// \return true   if all updates were confirmed by the broker.
    /// \return false  if update failed or waiting timed out
    rmqt::Result<>
    updateTopology(const rmqa::TopologyUpdate& topologyUpdate,
                   const bsls::TimeInterval& timeout = bsls::TimeInterval(0));

#ifdef USES_LIBRMQ_EXPERIMENTAL_FEATURES
    /// \brief Send a message with the given `routingKey` to the exchange
    /// targeted by the producer.
    ///
    /// The behavior of this method depends on the the number of unconfirmed
    /// messages (sent but not yet confirmed by the broker). If this number is
    /// smaller than the limit configured when calling
    /// rmqa::VHost#createProducer, this method behaves exactly as the method
    /// `send`. Otherwise, unlike `send`, this method returns immediately with
    /// a result indicating that the unconfirmed message limit has been
    /// reached.
    ///
    /// \param message         The message to be sent.
    /// \param routingKey      The routing key (e.g. topic or queue name)
    ///                        passed to the exchange.
    /// \param confirmCallback Called when the broker explicitly
    ///                        confirms/rejects
    ///                        the message. Messages are automatically retried
    ///                        on reconnection, in which case this method may
    ///                        be called some time after invoking `send`.
    ///
    /// \return SENDING        Returned when the library accepts the message
    ///                        for sending.
    ///                        If the connection is lost before receiving the
    ///                        publisher confirm from the broker, the library
    ///                        will retry sending the message.
    /// \return DUPLICATE      Returned if a message with the same GUID has
    ///                        already been sent and is awaiting a confirm from
    ///                        the broker. This indicates an issue with the
    ///                        application. To send the same message multiple
    ///                        times, a new rmqt::Message object must be
    ///                        created every time.
    /// \return INFLIGHT_LIMIT Returned if the unconfirmed message limit has
    ///                        been reached.
    rmqp::Producer::SendStatus
    trySend(const rmqt::Message& message,
            const bsl::string& routingKey,
            const rmqp::Producer::ConfirmationCallback& confirmCallback);
#endif

    /// Wait for all outstanding publisher confirms to arrive.
    ///
    /// \param timeout   How long to wait for. If timeout is 0, the method will
    ///                indefinitely wait for confirms.
    ///
    /// \return true   if all outstanding confirms have arrived.
    /// \return false  if waiting timed out.
    rmqt::Result<>
    waitForConfirms(const bsls::TimeInterval& timeout = bsls::TimeInterval());

  private:
    Producer(const Producer&) BSLS_KEYWORD_DELETED;
    Producer& operator=(const Producer&) BSLS_KEYWORD_DELETED;

  private:
    bslma::ManagedPtr<rmqp::Producer> d_impl;

}; // class Producer

} // namespace rmqa
} // namespace BloombergLP

#endif // ! INCLUDED_RMQA_PRODUCER
