/*
	DaSHEL
	A cross-platform DAta Stream Helper Encapsulation Library
	Copyright (C) 2007:
		
		Stephane Magnenat <stephane at magnenat dot net>
			(http://stephane.magnenat.net)
		Mobots group - Laboratory of Robotics Systems, EPFL, Lausanne
			(http://mobots.epfl.ch)
		
		Sebastian Gerlach
		Kenzan Technologies
			(http://www.kenzantech.com)
	
	All rights reserved.
	
	Redistribution and use in source and binary forms, with or without
	modification, are permitted provided that the following conditions are met:
		* Redistributions of source code must retain the above copyright
		  notice, this list of conditions and the following disclaimer.
		* Redistributions in binary form must reproduce the above copyright
		  notice, this list of conditions and the following disclaimer in the
		  documentation and/or other materials provided with the distribution.
		* Neither the names of "Mobots", "Laboratory of Robotics Systems", "EPFL",
		  "Kenzan Technologies" nor the names of the contributors may be used to
		  endorse or promote products derived from this software without specific
		  prior written permission.
	
	THIS SOFTWARE IS PROVIDED BY COPYRIGHT HOLDERS ``AS IS'' AND ANY
	EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
	WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
	DISCLAIMED. IN NO EVENT SHALL COPYRIGHT HOLDERS BE LIABLE FOR ANY
	DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
	(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
	LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
	ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
	(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
	SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef __DaSHEL_H
#define __DaSHEL_H

#include <string>
#include <set>
#include <map>

/*!	\file streams.h
	\brief Public interface of DaSHEL, A cross-platform DAta Stream Helper Encapsulation Library
*/

/**
	\mainpage Streams
	
	Stéphane Magnenat (http://stephane.magnenat.net),
	Mobots group - Laboratory of Robotics Systems, EPFL, Lausanne (http://mobots.epfl.ch) \n
	Sebastion Gerlach,
	Kenzan Technologies (http://www.kenzantech.com)
	
	\section IntroSec Introduction
	
	DaSHEL is a cross-platform DAta Stream Helper Encapsulation Library.
	It provides a unified access to TCP/IP, serial port, and files streams.
	It also allows a server application to wait for any activity
	on any combination of those streams.
	
	Streams is licensed under a modified BSD license, which is a permissive open source license.
	Yet, if you find bugs or do some improvements, please let us know.
	
	\section TargetNamingSec Targets Naming
	
	To construct a new Client, or to listen connections in a Hub, you have to specify a target.
	A target is a string which describes a file, a TCP/IP address/port, or a serial port.
	This string consists of the type of the target followed by a comma separated list of parameters.
	This list is contains key-values pair, with a predifined order such that keys can be omitted (but if a key is present, all subsequent entries must have an explicit key).
	Its general syntax is thus \c "protocol:[param1key=]param1value;...;[paramNkey=]paramNvalue".
	
	The following protocols are available:
	\li \c file : local files
	\li \c tcp : TCP/IP
	\li \c ser : serial port
	\li \c stdin : standard input
	\li \c stdout : standard output
	
	The file protocol accepts the following parameters, in this implicit order:
	\li \c name : name of the file, including the path
	\li \c mode : mode (read, write)
	
	The tcp protocol accepts the following parameters, in this implicit order:
	\li \c host : host
	\li \c port : port
	
	The ser protocol accepts the following parameters, in this implicit order:
	\li \c device : serial port device name, system specific; either port or device must be given, device has priority if both are given.
	\li \c port : serial port number, starting from 1, default 1; either port or device must be given, device has priority if both are given.
	\li \c baud : baud rate, default 115200
	\li \c stop : stop bits count (1 or 2), default 1
	\li \c parity : parity type (none, even, odd), default none
	\li \c fc : flow control type, (none, hard), default none
	\li \c bits : number of bits per character, default 8
	
	Protocols \c stdin and \c stdout do not take any parameter.
*/

//! DaSHEL, a cross-platform stream abstraction library
namespace Dashel
{
	class Stream;

	//! The one size fits all exception for streams
	class DashelException
	{
	public:
		//! The different exception causes.
		typedef enum {
			Unknown,			//!< Well, hopefully never used.
			SyncError,			//!< Some synchronisation error.
			InvalidTarget,		//!< The target string was bad.
			InvalidOperation,	//!< The operation is not valid on this stream.
			ConnectionLost,		//!< The connection was lost.
			IOError,			//!< Some I/O error.
			ConnectionFailed,	//!< The connection could not be established.
			EnumerationError,	//!< Some serial enumeration error
			PreviousIncomingDataNotRead //!< The incoming data was not read by the Hub subclass
		} Source;

		//! The exception cause.
		Source source;
		//! The reason as an OS error code.
		int sysError;
		//! The reason as a human readable string according to the OS.
		std::string sysMessage;
		//! The reason as a human readable string.
		std::string reason;
		//! The stream that caused the exception to be thrown.
		Stream *stream;

	public:
		//! Construct an stream exception with everything.
		/*!	\param s Source of failure
			\param se System error code.
			\param reason The logical reason as a human readable string.
			\param stream Stream to which exception applies.
		*/
		DashelException(Source s, int se, const char *reason, Stream* stream = NULL);
	};
	
	//! Serial port enumerator class.
	/*! This class is just a package for one static method.
	*/
	class SerialPortEnumerator
	{
	public:
		//! Retrieve list of all serial ports available on system.
		/*! This function queries the Operating System for all available serial ports.
			\return A map where the key is the port number name as passed to the ser: protocol, and
			the value is a pair of the system device name and a human readable description
			that may be displayed in a user interface.
		*/
		static std::map<int, std::pair<std::string, std::string> > getPorts();
	};

	//! A data stream, with low-level (not-endian safe) read/write functions
	class Stream
	{
	private:
		//! A flag indicating that the stream has failed.
		bool failedFlag;
		//! The human readable reason describing why the stream has failed.
		std::string failReason;

	protected:
		//! The target name.
		std::string targetName;

	public:
		//! Constructor.
		Stream(const std::string& targetName) { this->targetName = targetName; failedFlag = false; }

		//! Virtual destructor, to ensure calls to destructors of sub-classes.
		virtual ~Stream() {}

		//! Set stream to failed state
		/*!	\param s Source of failure
			\param se System error code
			\param reason The logical reason as a human readable string.
		*/
		void fail(DashelException::Source s, int se, const char* reason);

		//! Query failed state of stream.
		/*! \return true if stream has failed.
		*/
		bool failed() const { return failedFlag; }
		
		//!	Returns the reason the stream has failed.
		/*!	\return the reason the stream has failed, or an empty string if fail() is false.
		*/
		const std::string &getFailReason() const { return failReason; }
		
		//!	Returns the name of the target.
		/*!	The name of the target contains all parameters and the protocol name.

			\return Name of the target
		*/
		const std::string &getTargetName() const { return targetName; }
		
		//!	Write data to the stream.
		/*!	Writes all requested data to the stream, blocking until all the data has been written, or 
			some error occurs. Errors are signaled by throwing a DashelException exception. This function
			does not flush devices, therefore the data may not really have been written on return, but only
			been buffered. In order to flush the stream, call flush().
			
			\param data Pointer to the data to write.
			\param size Amount of data to write in bytes.
		*/
		virtual void write(const void *data, const size_t size) = 0;
		
		//! Write a variable of basic type to the stream
		/*! This function does not perform any endian conversion.
			\param v variable to write.
		*/
		template<typename T> void write(T v)
		{
			write(&v, sizeof(T));
		}
		
		//!	Flushes stream.
		/*!	Calling this function requests the stream to be flushed, this may ensure that data is written
			to physical media or actually sent over a wire. The exact performed function depends on the
			stream type and operating system.
		*/
		virtual void flush() = 0;
		
		//!	Reads data from the stream.
		/*!	Reads all requested data from the stream, blocking until all the data has been read, or 
			some error occurs. Errors are signaled by throwing a DashelException exception, which may
			be caused either by device errors or reaching the end of file. 
			
			\param data Pointer to the memory where the read data should be stored.
			\param size Amount of data to read in bytes.
		*/
		virtual void read(void *data, size_t size) = 0;
		
		//! Read a variable of basic type from the stream
		/*! This function does not perform any endian conversion.
		
			\return variable to read.
		*/
		template<typename T> T read()
		{
			T v;
			read(&v, sizeof(T));
			return v;
		}
	};
	
	/**
		A server that listens for incoming connections and maintains a list of
		targets.
		To create a client connection, users of the library have to subclass Hub
		and implement incomingConnection(), incomingData(), and connectionClosed().
	*/
	class Hub
	{
	public:
		//! A list of streams
		typedef std::set<Stream*> StreamsSet;
		
	private:
		void *hTerminate;			//!< Set when this thing goes down.
		StreamsSet streams; 		//!< All our streams.
	
	protected:
		StreamsSet dataStreams;		//!< All our streams that transfer data (in opposition to streams that just listen for data).
	
	public:
		//! Constructor.
		Hub();
	
		//! Destructor, closes all connections.
		virtual ~Hub();
		
		/**
			Listens for incoming connections on a target.
			Some targets, such as a serial ports and files may directly generate a new connection;
			others, such as network interfaces, will only generate news connections when a peer
			connects.
			May throw a ConnectionError exception if the target does not exists or is not ready.
			
			\param target destination to listen connections from (see Section \ref TargetNamingSec)
			\return the stream we are connected to; if connect was not possible, an exception was throw.
		*/
		Stream* connect(const std::string &target);
		
		/**
			Close a stream, remove it from the Hub, and delete it.
			If the stream is not present in the Hub, it is deleted nevertheless.
			Note that connectionClosed() is not called by closeStream() and that
			you must not call closeStream(s) from inside connectionClosed(s) for
			the same s.
			
			\param stream stream to remove
		*/
		void closeStream(Stream* stream);
		
		//! Runs and returns only when an external event requests the application to stop.
		void run(void);
		
		/**
			Waits for data from the transfers streams or connections from the listening streams.
			Read all available data.
		
			\param timeout if -1, waits until data arrive. If 0, do not wait, just poll for activity. If positive, waits at maximum timeout ms.
			\return false if stop() was called or the application was requested to terminate, true otherwise.
		*/
		bool step(int timeout = 0);

	protected:
		//! Stops running, subclasses may call this function.
		void stop();
		
		/**
			Called when any data connection is created.
			It is not called when a listening connection (eg tcpin:) is created.
			If the stream is closed during this method, an exception occurs: the caller is responsible to handle it.
			The stream is already inserted in the stream list when this function is called.
			Subclass can implement this method.
			
			\param stream stream to the target
		*/
		virtual void connectionCreated(Stream *stream) { }
		
		/**
			Called when data is available for reading on the stream.
			If the stream is closed during this method, an exception occurs: Hub stops the execution of this
			method and calls connectionClosed(); objects dynamically allocated must thus be handled
			with auto_ptr.
			If step() is used, subclass must implement this method and call read at least once.
			
			\param stream stream to the target
		*/
		virtual void incomingData(Stream *stream) { }
		
		/**
			Called when target closes connection.
			The only valid method to call on the stream is getTargetName(), input/output operations are forbidden.
			You must not call closeStream(stream) from within this method for the same stream as the
			one passed as parameter.
			Subclass can implement this method.
			
			\param stream stream to the target.
			\param abnormal whether the connection was closed during step (abnormal == false) or when an operation was performed (abnormal == true)
		*/
		virtual void connectionClosed(Stream *stream, bool abnormal) { }
	};
}

#endif