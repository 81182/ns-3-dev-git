/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2015 Natale Patriciello <natale.patriciello@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */
#ifndef PPDRTCLAN_H
#define PPDRTCLAN_H

/**
  * \mainpage Simulation platform for PPDR-TC EU Project
  *
  * \author Natale Patriciello (UNIMORE)
  * \author Martin Klapez (UNIMORE)
  * \author Carlo Augusto Grazia (UNIMORE)
  * \author Maurizio Casoni (UNIMORE)
  *
  * This work is done for PPDR-TC EU Project, in order to provide a tool for
  * simulating various network topology to get results in term of throughput,
  * end-to-end delay, jitter and quality of service experienced by ppdr users.
  *
  * To start, we recommend to follow this page, studying first how to make
  * simulation (in other words, get confidence with configuration files and
  * their format) and then, if needed, start studying the source code and
  * how the things are done.
  *
  * \section PageTOC Content
  *  -# \ref Description
  *   -# \ref Design
  *   -# \ref BlocksDescription
  *  -# \ref Configuration
  *  -# \ref Understanding
  *  -# \ref License
  *
  * \section Description Description
  *
  * The overall description of the simulator is presented here. Please note
  * that this is a derivate work of ns-3, and it is used as the core of this
  * simulator. Before getting in touch with the source code, we suggest
  * to study and understand at least the ns-3 tutorial and the basics of the
  * ns-3 manual (callback and traced values).
  *
  * \attention This is a work-in-progress. Check often, maybe things are changed
  * and you haven't noticed anything.
  *
  * \subsection Design Design
  *
  * The main purpose of the simulator is to allow a fast prototyping of
  * a ppdr network. By design, the main blocks are the following:
  * \verbatim

  -------           ------------              ----------
  |     |    LAN    |          |   Backhaul   |        |
  | LAN |  -------- | Gateways |--------------| Remote |
  |Hosts|   Network |          |   Network    | Hosts  |
  -------           ------------              ----------
  \endverbatim
  *
  * Each block can be configured independently from each other, and various
  * metrics can be enabled to measure the behaviour of the network. Lan hosts
  * are all ppdr client inside the ppdr network, connected through one (or more)
  * LAN networks to gateways, which provide connectivity to the outside world,
  * and, of course, remote hosts.
  * Briefly, this is an hypotetical network which can be derived with such
  * design:
  * \verbatim
   _
  |_|-----------
               |
              ____
   _         |    |                               _
  |_|--------|Gw1 | ---------------------------- |_|
             |____|\
   _            |   \                             _
  |_|------------    ----------------------------|_|

  \endverbatim
  *
  * Where three client are connecting through a gateway to two different remote
  * nodes. In the following subsection we will explain how to configure
  * each subsection and how these will interact.
  *
  * \subsection BlocksDescription Network Blocks Description
  *
  * The first section to consider when configuring the network is the General
  * one. Details about the section and its settings can be found in class
  * ppdrtc::GeneralSection. Its purpose is to have a place where all general
  * parameters (e.g. the name of the output files, simulation duration, and so
  * on) can be set and used in various blocks.
  *
  * For the LAN network, the relevant section to configure is ppdrtc::Lan. Here
  * you can specify the number of clients (expressed as integer number) and the
  * number of gateways of the network (configured through ppdrtc::GatewaySection,
  * which mimics what happen with client nodes and so it is similar). Another
  * important aspect is the type of the LAN network: LTE (configured through
  * ppdrtc::LteSection) or P2P. For the P2P network, the parameters are inserted
  * in each ppdrtc::ClientSection, as they can be different for different clients.
  * Last but not least, you should write down the network topology, in other words,
  * you should declare to what gateway each node will connect to. This has different
  * meanings for different types of network; please refer to the technical documentation
  * of the section for more information.
  *
  * To set the backhaul properties, there is the ppdrtc::BackhaulSection. At this
  * time, only p2p is supported, with the same values for all links between
  * gateways and remote. In other words, all remote nodes will be connected
  * to all the gateways with a p2p link.
  *
  * Each node can be configured by using a separate section of type
  * ppdrtc::ClientSection, ppdrtc::GatewaySection or ppdrtc::RemoteSection.
  * For instance, if you needs three clients (it doesn't
  * matter, at this stage, to what gateways they will be connected) the following
  * section should be added to the configuration file:
  * \verbatim

  [client0]
  ....

  [client1]
  ....

  [client2]
  ....
  \endverbatim
  *
  * Names are strictly connected to the roles of the node. So, you should give
  * the client name in the format "clienti" where i is a number between 0 and
  * client number - 1.
  *
  * Then, a series of section of type ppdrtc::RemoteSection and ppdrtc::GatewaySection
  * is expected (which represent the remote hosts) and finally a ppdrtc::StatisticsSection can be
  * written, in order to specify the desidered output data from the simulation.
  *
  * Please note that the order is not strict. You can write down the sections
  * as you want.
  *
  * \subsection Applications Applications on the PPDR network
  *
  * As you can imagine, the needs in term of applications are very different
  * from user to user. In order to let you simulate each different needs,
  * this program provides three main way to configure an exchange of data
  * between nodes.
  *
  * As a general remark, this exchange is made between two nodes, where on
  * each one is installed an application. One behave as a sink, the other as a
  * sender. So, the first basic application is a ppdrtc::AppSection. Then, you
  * can have an active bulk sender (continuosly sends data, ppdrtc::BulkSendSection)
  * or an application which follows an On/Off pattern (stay on and transmit, then
  * switch to off and stops transmitting, then go on, then off..) which is
  * ppdrtc::OnOffSection. The basic information about the application is where
  * it is installed on and where it needs to connect to.
  *
  * \section Output Output
  *
  * Various output can be enabled by appropriately configuring the ppdrtc::StatisticSection
  * section. The main class responsible for generating these output is the class
  * ppdrtc::Statistic, which the read of its documentation is strongly suggested
  * in order to actively using (and interpreting) the output of the simulation.
  *
  * BE MORE SPECIFIC PLEASE!
  *
  * \section Configuration Start using PPDR-TC Simulator
  *
  * Here it is an example configuration file:
  *
  *\verbatim
attach the example
  \endverbatim
  *
  * \section Understanding Understanding the code
  *
  * To understand the code, you need a little background on the ns-3
  * network simulator (which this work is derived from). Then, you can start
  * from the main() function.
  *
  * Three main files are used to create the entire ppdr-tc simulator. First,
  * you can find some helpers in the ppdr-tc-lan.cc file. The statistics are
  * calculated (and, in some cases, obtained) in ppdr-tc-statistics.cc (and
  * in the relative class Statistics). Classes and functions for configuring
  * the simulator are in ppdr-tc-utils.cc. Pre-defined scenarios can be found
  * in ppdr-tc-scenarios.cc.
  *
  * You will be guided into the source code in a logical way through the main()
  * function; if you need something specialized, you can use the search tool
  * provided in the upper right section of this page. For any further information,
  * please contact the copyright owner, keeping in mind that it isn't an help-desk
  * and NO WARRANTY is provided. Also, we are not sure of being able to reply
  * to all mail. In order to increase the chances, please format the e-mail
  * in a readable way (preferably plaintext) and try to be coincise. Proof-of-concept
  * code is always recommended, with an example of using. Comments are encouraged,
  * critics are accepted, and patches welcomed. Thank you for using this product.
  *
  * \section License License
  * \copyright (c) 2015 Natale Patriciello <natale.patriciello@gmail.com>
  *
  * This program is free software; you can redistribute it and/or modify
  * it under the terms of the GNU General Public License version 2 as
  * published by the Free Software Foundation;
  *
  * This program is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  * GNU General Public License for more details.
  *
  * You should have received a copy of the GNU General Public License
  * along with this program; if not, write to the Free Software
  * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
  */
#endif // PPDRTCLAN_H

