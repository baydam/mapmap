/*
 * OscInterface.cpp
 *
 * Copyright (c) 2010 Alexandre Quessy <alexandre@quessy.net>
 * Copyright (c) 2010 Tristan Matthews <le.businessman@gmail.com>
 * (c) 2013 Sofian Audry -- info(@)sofianaudry(.)com
 * (c) 2013 Alexandre Quessy -- alexandre(@)quessy(.)net
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "OscInterface.h"
#include "MainWindow.h"
#include <QVariant>

OscInterface::OscInterface(
        MainWindow* owner,
        const std::string &listen_port) :
    receiver_(listen_port),
    owner_(owner),
    messaging_queue_()
{
    //if (listen_port != OSC_PORT_NONE)
    receiving_enabled_ = true;
    if (receiving_enabled_)
    {
        std::cout << "Listening osc_udp://localhost:" << listen_port << std::endl;
        // receiver_.addHandler("/ping", "", ping_cb, this);
        // receiver_.addHandler("/pong", "", pong_cb, this);
        // receiver_.addHandler("/image/path", "s", image_path_cb, this);
        receiver_.addHandler(NULL, NULL, genericHandler, this);
    }
}

/**
 * Handles /pong. Does nothing.
 */
int OscInterface::pong_cb(
        const char *path,
        const char * /*types*/, 
        lo_arg ** /*argv*/,
        int /*argc*/, 
        void * /*data*/, 
        void *user_data)
{
    OscInterface* context = static_cast<OscInterface*>(user_data);
    if (context->is_verbose())
        std::cout << "Got " << path << std::endl;
    return 0;
}

/**
 * Handles /ping. Does nothing.
 */
int OscInterface::ping_cb(
        const char *path,
        const char * /*types*/, 
        lo_arg ** /*argv*/,
        int /*argc*/, 
        void * /*data*/, 
        void *user_data)
{
    OscInterface* context = static_cast<OscInterface*>(user_data);
    if (context->is_verbose())
        std::cout << "Got " << path << std::endl;
    return 0;
}

int OscInterface::image_path_cb(
        const char *path,
        const char * /*types*/, 
        lo_arg **argv,
        int /*argc*/, 
        void * /*data*/, 
        void *user_data)
{
    OscInterface* context = static_cast<OscInterface*>(user_data);
    std::string file_name(static_cast<const char*>(&argv[0]->s));
    //if (context->is_verbose())
        std::cout << "Got " << path << " " << file_name << std::endl;
    QVariantList message;
    message.append(QVariant(QString("image/path")));
    message.append(QVariant(QString(file_name.c_str())));
    context->push_command(message);
    return 0;
}

void OscInterface::push_command(QVariantList command)
{
    messaging_queue_.push(command);
}
/**
 * Takes action!
 *
 * Should be called when it's time to take action, before rendering a frame, for example.
 */
void OscInterface::consume_commands()
{
    bool success = true;
    while (success)
    {
        QVariantList command;
        success = messaging_queue_.try_pop(command);
        if (success)
        {
            //if (is_verbose())
            // std::cout << __FUNCTION__ << ": apply " <<
            //     command.first().toString().toStdString() << std::endl;
            owner_->applyOscCommand(command);
        }
    }
}

/** Starts listening if enabled
 */
void OscInterface::start()
{
    if (receiving_enabled_)
    {
        // start a thread to try and subscribe us
        receiver_.listen(); // start listening in separate thread
    }
}

/* catch any incoming messages and display them. returning 1 means that the 
 *  * message has not been fully handled and the server should try other methods */
int OscInterface::genericHandler(const char *path, 
        const char *types, lo_arg **argv, 
        int argc, void * /*data*/, void * user_data) 
{
  OscInterface* context = static_cast<OscInterface*>(user_data);
  QVariantList message;

  message.append(QVariant(QString(path)));
  message.append(QVariant(QString(types)));

  for (int i = 0; i < argc; ++i) 
  { 
    switch (types[i])
    {
      case 'i':
        message.append(QVariant(argv[i]->i));
        break;
      case 'f':
        message.append(QVariant((double) argv[i]->f));
        break;
      case 's':
        message.append(QVariant(QString(static_cast<const char *>(&argv[i]->s))));
        break;
      case 'd':
        message.append(QVariant((double) argv[i]->d));
        break;
      case 'T':
        message.append(QVariant(true));
        break;
      case 'F':
        message.append(QVariant(false));
        break;
      default:
        break;
    }
    context->push_command(message);
  } 
  return 0; // handled
}