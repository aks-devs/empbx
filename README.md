<p>
    A lightweight soft PBX based on Baresip and libre. <br>
    <br>
    The idea to have such software on the hand, has been in my mind for a long time... <br>
    Started developing projects such as: tSIP, HATS, I realized that, I don't have a solution that I could use as a basis for operate with VoIP. <br>
    Freeswitch was not suitable in this case, since it had to be small, portable and ready for emedding as possible. <br>
    The Baresip (as I usually used it) didn't suit this time. <br>
    So, I decided it was time to write something that would satisfy my needs... <br>
</p>

I've defined the following requirements: 
<ul>
 <li><strong>Small, portable and configurable</strong>
	<p>
    	It should be able to work on devices with 512Mb RAM (such as: OrangePI One or MTK SoCs MT7621, as possible) <br>
        Capable with: OpenWRT and FreeBSD <br>
        Should allow to use as a basis for developing VoIP applications or something related with it
    </p>
 </li>
 <li><strong>Supports protocols:</strong>
  <p>
     - SIP (must be) <br>
     - WebRTC (optional) <br>
   </p>
  </li>
 <li><strong>Telephony capabilities:</strong>
  <p>
    - SIP registrar (for local users) <br>
    - SIP gateways (for: FXO/FXS equipments and cloud PBX) <br>
    - Audio and video (as possible) calls <br>
    - Calls routing, something similar to FreeSWITCH dialplan but simpler.
  </p>
 </li>
 <li><strong>Management capabilities:</strong>
  <p>
    - Web based management interface (optional) <br>
    - JSON-RPC services for automation and integration purposes
    </p>
 </li>
</ul>

	
### v0.0.1
 This is an experimental version, for testing purposes only. <br>
 Implemented capabilities: <br> 
  - SIP registrar and location <br>
  - SIP gateways <br>  
  - Inbound/Outbound/Intrecom calls <br>
  - Dialplan with applications <br>
  - Supported codeds: G711, G729 <br>
  - Local users

### Build and run
 Just perform script: build.sh, and wait a bit. <br>
 Default home path: /opt/empbx <br>
 If everything compiles without errors you can go there and try to start it: ./bin/empbxd -a debug <br>
 There is a single config file: /opt/empbx/configs/empbxd-conf.xml, you can describe users, gateways, dialplan and etc, there. <br>
 



