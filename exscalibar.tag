<?xml version='1.0' encoding='ISO-8859-1' standalone='yes'?>
<tagfile>
  <compound kind="class">
    <name>Monitor</name>
    <filename>classMonitor.html</filename>
    <base>Geddei::Processor</base>
    <member kind="function">
      <type>const double</type>
      <name>averageThroughput</name>
      <anchor>a0</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>const double</type>
      <name>elapsedTime</name>
      <anchor>a1</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>const uint</type>
      <name>elementsProcessed</name>
      <anchor>a2</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>resetPlungerMemory</name>
      <anchor>a4</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>const uint</type>
      <name>samplesProcessed</name>
      <anchor>a5</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>const double</type>
      <name>signalProcessed</name>
      <anchor>a6</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>const double</type>
      <name>timesFasterThanReal</name>
      <anchor>a7</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>waitForPlunger</name>
      <anchor>a8</anchor>
      <arglist>()</arglist>
    </member>
  </compound>
  <compound kind="class">
    <name>QFactoryManager</name>
    <filename>classQFactoryManager.html</filename>
    <templarg>Base</templarg>
    <member kind="function">
      <type>Base *</type>
      <name>createInstance</name>
      <anchor>a0</anchor>
      <arglist>(const QString &amp;id)</arglist>
    </member>
    <member kind="function">
      <type>const QStringList &amp;</type>
      <name>getAvailable</name>
      <anchor>a1</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>const int</type>
      <name>getVersion</name>
      <anchor>a2</anchor>
      <arglist>(const QString &amp;id)</arglist>
    </member>
    <member kind="function">
      <type>const bool</type>
      <name>isAvailable</name>
      <anchor>a3</anchor>
      <arglist>(const QString &amp;id)</arglist>
    </member>
    <member kind="function">
      <type>Base *</type>
      <name>operator[]</name>
      <anchor>a4</anchor>
      <arglist>(const QString &amp;id)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>reloadAll</name>
      <anchor>a6</anchor>
      <arglist>(const QStringList &amp;thePaths)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>reloadAll</name>
      <anchor>a7</anchor>
      <arglist>(const QString &amp;thePath)</arglist>
    </member>
  </compound>
  <compound kind="class">
    <name>QSocketSession</name>
    <filename>classQSocketSession.html</filename>
    <member kind="function">
      <type></type>
      <name>QSocketSession</name>
      <anchor>a0</anchor>
      <arglist>(QSocketDevice *sd)</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>~QSocketSession</name>
      <anchor>a1</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>ack</name>
      <anchor>z30_0</anchor>
      <arglist>(const bool sign=true)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>close</name>
      <anchor>z30_1</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>const bool</type>
      <name>isOpen</name>
      <anchor>z30_2</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>const uchar</type>
      <name>receiveByte</name>
      <anchor>z30_3</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>const bool</type>
      <name>receiveChunk</name>
      <anchor>z30_4</anchor>
      <arglist>(uchar *buffer, const uint size, const uint timeOut)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>receiveChunk</name>
      <anchor>z30_5</anchor>
      <arglist>(uchar *buffer, const uint size)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>sendByte</name>
      <anchor>z30_6</anchor>
      <arglist>(const uchar c)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>sendChunk</name>
      <anchor>z30_7</anchor>
      <arglist>(const uchar *buffer, const uint size)</arglist>
    </member>
    <member kind="function">
      <type>const bool</type>
      <name>waitForAck</name>
      <anchor>z30_8</anchor>
      <arglist>(const uint timeOut, bool *ackType=0)</arglist>
    </member>
    <member kind="function">
      <type>const bool</type>
      <name>waitForAck</name>
      <anchor>z30_9</anchor>
      <arglist>(bool *ackType=0)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>handshake</name>
      <anchor>z32_0</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>handshake</name>
      <anchor>z32_1</anchor>
      <arglist>(const bool opposite)</arglist>
    </member>
    <member kind="function">
      <type>const QCString</type>
      <name>receiveString</name>
      <anchor>z32_2</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>const T</type>
      <name>safeReceiveWord</name>
      <anchor>z32_3</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>safeReceiveWordArray</name>
      <anchor>z32_4</anchor>
      <arglist>(T *t, const uint size)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>safeSendWord</name>
      <anchor>z32_5</anchor>
      <arglist>(const T i)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>safeSendWordArray</name>
      <anchor>z32_6</anchor>
      <arglist>(const T *i, const uint size)</arglist>
    </member>
    <member kind="function">
      <type>const bool</type>
      <name>sameByteOrder</name>
      <anchor>z32_7</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>sendString</name>
      <anchor>z32_8</anchor>
      <arglist>(const QCString &amp;s)</arglist>
    </member>
  </compound>
  <compound kind="class">
    <name>QSubApp</name>
    <filename>classQSubApp.html</filename>
    <member kind="function" static="yes">
      <type>void</type>
      <name>sleep</name>
      <anchor>e0</anchor>
      <arglist>(const uint secs)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>void</type>
      <name>usleep</name>
      <anchor>e1</anchor>
      <arglist>(const uint usecs)</arglist>
    </member>
    <member kind="function" protection="protected" virtualness="pure">
      <type>virtual void</type>
      <name>main</name>
      <anchor>b0</anchor>
      <arglist>()=0</arglist>
    </member>
    <member kind="function" protection="protected" virtualness="pure">
      <type>virtual void</type>
      <name>main</name>
      <anchor>b1</anchor>
      <arglist>()=0</arglist>
    </member>
    <member kind="function" protection="protected">
      <type></type>
      <name>QSubApp</name>
      <anchor>b2</anchor>
      <arglist>()</arglist>
    </member>
  </compound>
  <compound kind="class">
    <name>Recorder</name>
    <filename>classRecorder.html</filename>
    <base>Geddei::Processor</base>
    <member kind="function">
      <type></type>
      <name>Recorder</name>
      <anchor>a0</anchor>
      <arglist>()</arglist>
    </member>
  </compound>
  <compound kind="group">
    <name>Geddei</name>
    <title>Geddei&apos;s core classes.</title>
    <filename>group__Geddei.html</filename>
    <class kind="class">Geddei::BasicCreator</class>
    <class kind="class">Geddei::BasicSubCreator</class>
    <class kind="class">Geddei::BufferData</class>
    <class kind="class">Geddei::BufferDatas</class>
    <class kind="class">Geddei::DomProcessor</class>
    <class kind="class">Geddei::FactoryCreator</class>
    <class kind="class">Geddei::LxConnection</class>
    <class kind="class">Geddei::MultiProcessor</class>
    <class kind="class">Geddei::MultiProcessorCreator</class>
    <class kind="class">Geddei::Processor</class>
    <class kind="class">Geddei::ProcessorFactory</class>
    <class kind="class">Geddei::ProcessorGroup</class>
    <class kind="class">Geddei::ProcessorPort</class>
    <class kind="class">Geddei::Properties</class>
    <class kind="class">Geddei::PropertiesInfo</class>
    <class kind="class">Geddei::SignalType</class>
    <class kind="class">Geddei::SignalTypeRef</class>
    <class kind="class">Geddei::SignalTypeRefs</class>
    <class kind="class">Geddei::SubFactoryCreator</class>
    <class kind="class">Geddei::SubProcessor</class>
    <class kind="class">Geddei::SubProcessorFactory</class>
    <class kind="class">Geddei::xLConnection</class>
  </compound>
  <compound kind="group">
    <name>SignalTypes</name>
    <title>Classes derived from SignalType.</title>
    <filename>group__SignalTypes.html</filename>
    <class kind="class">SignalTypes::Matrix</class>
    <class kind="class">SignalTypes::Spectrum</class>
    <class kind="class">SignalTypes::SquareMatrix</class>
    <class kind="class">SignalTypes::Value</class>
    <class kind="class">SignalTypes::Wave</class>
  </compound>
  <compound kind="group">
    <name>Toolkit</name>
    <title>Extended Processor classes to help analyse networks.</title>
    <filename>group__Toolkit.html</filename>
    <class kind="class">Monitor</class>
    <class kind="class">Recorder</class>
  </compound>
  <compound kind="group">
    <name>QtExtra</name>
    <title>Classes to further consolidate Qt as the ultimate cross platform framework.</title>
    <filename>group__QtExtra.html</filename>
    <class kind="class">QFactoryManager</class>
    <class kind="class">QSocketSession</class>
    <class kind="class">QSubApp</class>
  </compound>
  <compound kind="group">
    <name>rGeddei</name>
    <title>rGeddei&apos;s core classes.</title>
    <filename>group__rGeddei.html</filename>
    <class kind="class">rGeddei::AbstractDomProcessor</class>
    <class kind="class">rGeddei::AbstractProcessor</class>
    <class kind="class">rGeddei::AbstractProcessorGroup</class>
    <class kind="class">rGeddei::AbstractProcessorPort</class>
    <class kind="class">rGeddei::LocalDomProcessor</class>
    <class kind="class">rGeddei::LocalProcessor</class>
    <class kind="class">rGeddei::LocalSession</class>
    <class kind="class">rGeddei::RemoteDomProcessor</class>
    <class kind="class">rGeddei::RemoteProcessor</class>
    <class kind="class">rGeddei::RemoteSession</class>
  </compound>
  <compound kind="class">
    <name>Geddei::BasicCreator</name>
    <filename>classGeddei_1_1BasicCreator.html</filename>
    <templarg>X</templarg>
    <base>Geddei::MultiProcessorCreator</base>
  </compound>
  <compound kind="class">
    <name>Geddei::BasicSubCreator</name>
    <filename>classGeddei_1_1BasicSubCreator.html</filename>
    <templarg>X</templarg>
    <base>Geddei::MultiProcessorCreator</base>
  </compound>
  <compound kind="class">
    <name>Geddei::BufferData</name>
    <filename>classGeddei_1_1BufferData.html</filename>
    <member kind="function">
      <type></type>
      <name>BufferData</name>
      <anchor>a0</anchor>
      <arglist>(const BufferData &amp;source)</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>BufferData</name>
      <anchor>a1</anchor>
      <arglist>(float *data, const uint size, const uint scope=1)</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>BufferData</name>
      <anchor>a2</anchor>
      <arglist>(const float *data, const uint size, const uint scope=1)</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>BufferData</name>
      <anchor>a3</anchor>
      <arglist>(const uint size, const uint scope=1)</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>BufferData</name>
      <anchor>a4</anchor>
      <arglist>(const bool valid=false)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>copyFrom</name>
      <anchor>a5</anchor>
      <arglist>(const float *source)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>copyFrom</name>
      <anchor>a6</anchor>
      <arglist>(const BufferData &amp;source)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>copyTo</name>
      <anchor>a7</anchor>
      <arglist>(float *destination) const</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>debugInfo</name>
      <anchor>a8</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function">
      <type>const uint</type>
      <name>elements</name>
      <anchor>a9</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>endWritePointer</name>
      <anchor>a10</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>const BufferInfo *</type>
      <name>info</name>
      <anchor>a11</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function">
      <type>BufferInfo *</type>
      <name>info</name>
      <anchor>a12</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>const bool</type>
      <name>isNull</name>
      <anchor>a13</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function">
      <type>const bool</type>
      <name>isValid</name>
      <anchor>a14</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function">
      <type>BufferData</type>
      <name>mid</name>
      <anchor>a15</anchor>
      <arglist>(const uint start, const uint length)</arglist>
    </member>
    <member kind="function">
      <type>const BufferData</type>
      <name>mid</name>
      <anchor>a16</anchor>
      <arglist>(const uint start, const uint length) const</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>nullify</name>
      <anchor>a17</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>float &amp;</type>
      <name>operator()</name>
      <anchor>a18</anchor>
      <arglist>(const uint i, const uint j)</arglist>
    </member>
    <member kind="function">
      <type>const float &amp;</type>
      <name>operator()</name>
      <anchor>a19</anchor>
      <arglist>(const uint i, const uint j) const</arglist>
    </member>
    <member kind="function">
      <type>BufferData &amp;</type>
      <name>operator=</name>
      <anchor>a20</anchor>
      <arglist>(const BufferData &amp;source)</arglist>
    </member>
    <member kind="function">
      <type>const float &amp;</type>
      <name>operator[]</name>
      <anchor>a21</anchor>
      <arglist>(const uint i) const</arglist>
    </member>
    <member kind="function">
      <type>float &amp;</type>
      <name>operator[]</name>
      <anchor>a22</anchor>
      <arglist>(const uint i)</arglist>
    </member>
    <member kind="function">
      <type>const bool</type>
      <name>plunger</name>
      <anchor>a23</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function">
      <type>const float *</type>
      <name>readPointer</name>
      <anchor>a24</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function">
      <type>const uint</type>
      <name>reference</name>
      <anchor>a25</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function">
      <type>BufferData</type>
      <name>sample</name>
      <anchor>a26</anchor>
      <arglist>(const uint index)</arglist>
    </member>
    <member kind="function">
      <type>const BufferData</type>
      <name>sample</name>
      <anchor>a27</anchor>
      <arglist>(const uint index) const</arglist>
    </member>
    <member kind="function">
      <type>BufferData</type>
      <name>samples</name>
      <anchor>a28</anchor>
      <arglist>(const uint index, const uint amount)</arglist>
    </member>
    <member kind="function">
      <type>const BufferData</type>
      <name>samples</name>
      <anchor>a29</anchor>
      <arglist>(const uint index, const uint amount) const</arglist>
    </member>
    <member kind="function">
      <type>const uint</type>
      <name>samples</name>
      <anchor>a30</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function">
      <type>const uint</type>
      <name>scope</name>
      <anchor>a31</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function">
      <type>float *</type>
      <name>writePointer</name>
      <anchor>a32</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>~BufferData</name>
      <anchor>a33</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function" static="yes">
      <type>BufferData &amp;</type>
      <name>fake</name>
      <anchor>e0</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="friend" protection="private">
      <type>friend ostream &amp;</type>
      <name>operator&lt;&lt;</name>
      <anchor>n0</anchor>
      <arglist>(ostream &amp;out, const BufferData &amp;me)</arglist>
    </member>
  </compound>
  <compound kind="class">
    <name>Geddei::BufferDatas</name>
    <filename>classGeddei_1_1BufferDatas.html</filename>
    <member kind="function">
      <type></type>
      <name>BufferDatas</name>
      <anchor>a0</anchor>
      <arglist>(const uint count=0)</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>BufferDatas</name>
      <anchor>a1</anchor>
      <arglist>(const BufferDatas &amp;src)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>copyData</name>
      <anchor>a2</anchor>
      <arglist>(const uint i, const BufferData &amp;d)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>nullify</name>
      <anchor>a3</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>BufferDatas &amp;</type>
      <name>operator=</name>
      <anchor>a4</anchor>
      <arglist>(const BufferDatas &amp;src)</arglist>
    </member>
    <member kind="function">
      <type>BufferData &amp;</type>
      <name>operator[]</name>
      <anchor>a5</anchor>
      <arglist>(uint i)</arglist>
    </member>
    <member kind="function">
      <type>const BufferData &amp;</type>
      <name>operator[]</name>
      <anchor>a6</anchor>
      <arglist>(uint i) const</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>resize</name>
      <anchor>a7</anchor>
      <arglist>(const uint count)</arglist>
    </member>
    <member kind="function">
      <type>BufferDatas</type>
      <name>samples</name>
      <anchor>a8</anchor>
      <arglist>(const uint index, const uint amount)</arglist>
    </member>
    <member kind="function">
      <type>const BufferDatas</type>
      <name>samples</name>
      <anchor>a9</anchor>
      <arglist>(const uint index, const uint amount) const</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>setData</name>
      <anchor>a10</anchor>
      <arglist>(const uint i, const BufferData *d)</arglist>
    </member>
    <member kind="function">
      <type>const uint</type>
      <name>size</name>
      <anchor>a11</anchor>
      <arglist>() const</arglist>
    </member>
  </compound>
  <compound kind="class">
    <name>Geddei::DomProcessor</name>
    <filename>classGeddei_1_1DomProcessor.html</filename>
    <base>Geddei::Processor</base>
    <member kind="function">
      <type>void</type>
      <name>addWorker</name>
      <anchor>a0</anchor>
      <arglist>(SubProcessor *worker)</arglist>
    </member>
    <member kind="function">
      <type>const bool</type>
      <name>createAndAddWorker</name>
      <anchor>a1</anchor>
      <arglist>(const QString &amp;host, const uint key)</arglist>
    </member>
    <member kind="function">
      <type>const bool</type>
      <name>createAndAddWorker</name>
      <anchor>a2</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>DomProcessor</name>
      <anchor>a3</anchor>
      <arglist>(const QString &amp;primaryType)</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>DomProcessor</name>
      <anchor>a4</anchor>
      <arglist>(SubProcessor *primary)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>ratify</name>
      <anchor>a5</anchor>
      <arglist>(DxCoupling *c)</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual</type>
      <name>~DomProcessor</name>
      <anchor>a6</anchor>
      <arglist>()</arglist>
    </member>
  </compound>
  <compound kind="class">
    <name>Geddei::FactoryCreator</name>
    <filename>classGeddei_1_1FactoryCreator.html</filename>
    <base>Geddei::MultiProcessorCreator</base>
    <member kind="function">
      <type></type>
      <name>FactoryCreator</name>
      <anchor>a0</anchor>
      <arglist>(const QString &amp;type)</arglist>
    </member>
  </compound>
  <compound kind="class">
    <name>Geddei::LxConnection</name>
    <filename>classGeddei_1_1LxConnection.html</filename>
    <member kind="function" virtualness="virtual">
      <type>virtual BufferData</type>
      <name>makeScratchSample</name>
      <anchor>a0</anchor>
      <arglist>(bool autoPush=false)</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual BufferData</type>
      <name>makeScratchSamples</name>
      <anchor>a1</anchor>
      <arglist>(const uint samples, bool autoPush=false)</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual BufferData</type>
      <name>makeScratchSecond</name>
      <anchor>a2</anchor>
      <arglist>(bool autoPush=false)</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual BufferData</type>
      <name>makeScratchSeconds</name>
      <anchor>a3</anchor>
      <arglist>(const float seconds, bool autoPush=false)</arglist>
    </member>
    <member kind="function" virtualness="pure">
      <type>virtual const uint</type>
      <name>maximumScratchElements</name>
      <anchor>a4</anchor>
      <arglist>(const uint minimum=1)=0</arglist>
    </member>
    <member kind="function" virtualness="pure">
      <type>virtual const uint</type>
      <name>maximumScratchElementsEver</name>
      <anchor>a5</anchor>
      <arglist>()=0</arglist>
    </member>
    <member kind="function">
      <type>const uint</type>
      <name>maximumScratchSamples</name>
      <anchor>a6</anchor>
      <arglist>(const uint minimum=1)</arglist>
    </member>
    <member kind="function">
      <type>const uint</type>
      <name>maximumScratchSamplesEver</name>
      <anchor>a7</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>BufferData</type>
      <name>operator+</name>
      <anchor>a8</anchor>
      <arglist>(const uint samples)</arglist>
    </member>
    <member kind="function">
      <type>LxConnection &amp;</type>
      <name>operator&lt;&lt;</name>
      <anchor>a9</anchor>
      <arglist>(const BufferData &amp;data)</arglist>
    </member>
    <member kind="function" virtualness="pure">
      <type>virtual void</type>
      <name>push</name>
      <anchor>a10</anchor>
      <arglist>(const BufferData &amp;data)=0</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual const SignalTypeRef</type>
      <name>type</name>
      <anchor>a11</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>~LxConnection</name>
      <anchor>a12</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function" protection="protected">
      <type></type>
      <name>LxConnection</name>
      <anchor>b0</anchor>
      <arglist>(Source *source, const uint sourceIndex)</arglist>
    </member>
    <member kind="function" protection="protected" virtualness="virtual">
      <type>virtual BufferData</type>
      <name>makeScratchElements</name>
      <anchor>b1</anchor>
      <arglist>(const uint elements, bool autoPush=false)</arglist>
    </member>
    <member kind="variable" protection="protected">
      <type>Source *</type>
      <name>theSource</name>
      <anchor>p0</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable" protection="protected">
      <type>uint</type>
      <name>theSourceIndex</name>
      <anchor>p1</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="class">
    <name>Geddei::MultiProcessor</name>
    <filename>classGeddei_1_1MultiProcessor.html</filename>
    <member kind="function">
      <type>void</type>
      <name>init</name>
      <anchor>a0</anchor>
      <arglist>(const QString &amp;name=&quot;&quot;, const Properties &amp;properties=Properties())</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>init</name>
      <anchor>a1</anchor>
      <arglist>(const QString &amp;name, ProcessorGroup &amp;group, const Properties &amp;properties=Properties())</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>MultiProcessor</name>
      <anchor>a2</anchor>
      <arglist>(MultiProcessorCreator *c)</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>~MultiProcessor</name>
      <anchor>a3</anchor>
      <arglist>()</arglist>
    </member>
  </compound>
  <compound kind="class">
    <name>Geddei::MultiProcessorCreator</name>
    <filename>classGeddei_1_1MultiProcessorCreator.html</filename>
  </compound>
  <compound kind="class">
    <name>Geddei::Processor</name>
    <filename>classGeddei_1_1Processor.html</filename>
    <member kind="function" protection="protected">
      <type>const uint</type>
      <name>multiplicity</name>
      <anchor>z10_0</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="enumvalue">
      <name>Guarded</name>
      <anchor>w0w3</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumeration">
      <name>ErrorType</name>
      <anchor>w2</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>NoError</name>
      <anchor>w2w7</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>Pending</name>
      <anchor>w2w10</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>Custom</name>
      <anchor>w2w1</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>InputTypeNull</name>
      <anchor>w2w6</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>InputNotConnected</name>
      <anchor>w2w4</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>InputsNotHomogeneous</name>
      <anchor>w2w5</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>OutputsNull</name>
      <anchor>w2w9</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>RecursiveFailure</name>
      <anchor>w2w11</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>NotStarted</name>
      <anchor>w2w8</anchor>
      <arglist></arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual</type>
      <name>~Processor</name>
      <anchor>a0</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>const float</type>
      <name>bufferCapacity</name>
      <anchor>z21_0</anchor>
      <arglist>(const uint index)</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual const bool</type>
      <name>confirmTypes</name>
      <anchor>z21_1</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>const Connection *</type>
      <name>connect</name>
      <anchor>z21_2</anchor>
      <arglist>(const uint sourceIndex, const QString &amp;sinkHost, const uint sinkKey, const QString &amp;sinkProcessorName, const uint sinkIndex, const uint bufferSize=1)</arglist>
    </member>
    <member kind="function">
      <type>const Connection *</type>
      <name>connect</name>
      <anchor>z21_3</anchor>
      <arglist>(const uint sourceIndex, Sink *sink, const uint sinkIndex, const uint bufferSize=1)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>disconnect</name>
      <anchor>z21_4</anchor>
      <arglist>(const uint index)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>disconnectAll</name>
      <anchor>z21_5</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>draw</name>
      <anchor>z21_6</anchor>
      <arglist>(QPainter &amp;p)</arglist>
    </member>
    <member kind="function">
      <type>const QString</type>
      <name>error</name>
      <anchor>z21_7</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function">
      <type>const int</type>
      <name>errorData</name>
      <anchor>z21_8</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function">
      <type>const ErrorType</type>
      <name>errorType</name>
      <anchor>z21_9</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function">
      <type>const bool</type>
      <name>go</name>
      <anchor>z21_10</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>const uint</type>
      <name>height</name>
      <anchor>z21_11</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>inform</name>
      <anchor>z21_12</anchor>
      <arglist>(Processor &amp;target)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>init</name>
      <anchor>z21_13</anchor>
      <arglist>(const QString &amp;name=&quot;&quot;, const Properties &amp;properties=Properties())</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>init</name>
      <anchor>z21_14</anchor>
      <arglist>(const QString &amp;name, ProcessorGroup &amp;group, const Properties &amp;properties=Properties())</arglist>
    </member>
    <member kind="function">
      <type>const QString &amp;</type>
      <name>name</name>
      <anchor>z21_15</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function">
      <type>const uint</type>
      <name>numInputs</name>
      <anchor>z21_16</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function">
      <type>const uint</type>
      <name>numOutputs</name>
      <anchor>z21_17</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function">
      <type>ProcessorPort</type>
      <name>operator[]</name>
      <anchor>z21_18</anchor>
      <arglist>(const uint port)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>pause</name>
      <anchor>z21_19</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>const bool</type>
      <name>paused</name>
      <anchor>z21_20</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function">
      <type>const PropertiesInfo</type>
      <name>properties</name>
      <anchor>z21_21</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function">
      <type>const uint</type>
      <name>redrawPeriod</name>
      <anchor>z21_22</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>reset</name>
      <anchor>z21_23</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>setGroup</name>
      <anchor>z21_24</anchor>
      <arglist>(ProcessorGroup &amp;g)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>setNoGroup</name>
      <anchor>z21_25</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>share</name>
      <anchor>z21_26</anchor>
      <arglist>(const uint sourceIndex, const uint bufferSize=1)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>split</name>
      <anchor>z21_27</anchor>
      <arglist>(const uint sourceIndex)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>stop</name>
      <anchor>z21_28</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>const QString &amp;</type>
      <name>type</name>
      <anchor>z21_29</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>unpause</name>
      <anchor>z21_30</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>waitUntilDone</name>
      <anchor>z21_31</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>const ErrorType</type>
      <name>waitUntilGoing</name>
      <anchor>z21_32</anchor>
      <arglist>(int *errorData=0)</arglist>
    </member>
    <member kind="function">
      <type>const uint</type>
      <name>width</name>
      <anchor>z21_33</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function" static="yes">
      <type>Processor *</type>
      <name>getCallersProcessor</name>
      <anchor>e0</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function" protection="protected">
      <type>void</type>
      <name>bail</name>
      <anchor>b0</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function" protection="protected" virtualness="virtual">
      <type>virtual void</type>
      <name>haveStoppedNow</name>
      <anchor>b1</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function" protection="protected" virtualness="virtual">
      <type>virtual void</type>
      <name>wantToStopNow</name>
      <anchor>b2</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function" protection="protected" virtualness="pure">
      <type>virtual void</type>
      <name>initFromProperties</name>
      <anchor>z19_0</anchor>
      <arglist>(const Properties &amp;properties)=0</arglist>
    </member>
    <member kind="function" protection="protected">
      <type>xLConnection &amp;</type>
      <name>input</name>
      <anchor>z19_1</anchor>
      <arglist>(const uint index)</arglist>
    </member>
    <member kind="function" protection="protected">
      <type>LxConnection &amp;</type>
      <name>output</name>
      <anchor>z19_2</anchor>
      <arglist>(const uint index)</arglist>
    </member>
    <member kind="function" protection="protected" virtualness="virtual">
      <type>virtual void</type>
      <name>paintProcessor</name>
      <anchor>z19_3</anchor>
      <arglist>(QPainter &amp;p)</arglist>
    </member>
    <member kind="function" protection="protected">
      <type>void</type>
      <name>plunge</name>
      <anchor>z19_4</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function" protection="protected">
      <type></type>
      <name>Processor</name>
      <anchor>z19_5</anchor>
      <arglist>(const QString &amp;type, const MultiplicityType multi=NotMulti, const uint flags=0)</arglist>
    </member>
    <member kind="function" protection="protected" virtualness="pure">
      <type>virtual void</type>
      <name>processor</name>
      <anchor>z19_6</anchor>
      <arglist>()=0</arglist>
    </member>
    <member kind="function" protection="protected" virtualness="virtual">
      <type>virtual const bool</type>
      <name>processorStarted</name>
      <anchor>z19_7</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function" protection="protected" virtualness="virtual">
      <type>virtual void</type>
      <name>processorStopped</name>
      <anchor>z19_8</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function" protection="protected" virtualness="virtual">
      <type>virtual void</type>
      <name>receivedPlunger</name>
      <anchor>z19_9</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function" protection="protected">
      <type>void</type>
      <name>setupIO</name>
      <anchor>z19_10</anchor>
      <arglist>(const uint inputs, const uint outputs)</arglist>
    </member>
    <member kind="function" protection="protected">
      <type>void</type>
      <name>setupVisual</name>
      <anchor>z19_11</anchor>
      <arglist>(const uint width=50, const uint height=30, const uint redrawPeriod=0)</arglist>
    </member>
    <member kind="function" protection="protected" virtualness="virtual">
      <type>virtual void</type>
      <name>specifyInputSpace</name>
      <anchor>z19_12</anchor>
      <arglist>(QValueVector&lt; uint &gt; &amp;samples)</arglist>
    </member>
    <member kind="function" protection="protected" virtualness="virtual">
      <type>virtual void</type>
      <name>specifyOutputSpace</name>
      <anchor>z19_13</anchor>
      <arglist>(QValueVector&lt; uint &gt; &amp;samples)</arglist>
    </member>
    <member kind="function" protection="protected" virtualness="virtual">
      <type>virtual PropertiesInfo</type>
      <name>specifyProperties</name>
      <anchor>z19_14</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function" protection="protected">
      <type>const bool</type>
      <name>thereIsInputForProcessing</name>
      <anchor>z19_15</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function" protection="protected">
      <type>const bool</type>
      <name>thereIsInputForProcessing</name>
      <anchor>z19_16</anchor>
      <arglist>(const uint samples)</arglist>
    </member>
    <member kind="function" protection="protected" virtualness="pure">
      <type>virtual const bool</type>
      <name>verifyAndSpecifyTypes</name>
      <anchor>z19_17</anchor>
      <arglist>(const SignalTypeRefs &amp;inTypes, SignalTypeRefs &amp;outTypes)=0</arglist>
    </member>
  </compound>
  <compound kind="class">
    <name>Geddei::ProcessorFactory</name>
    <filename>classGeddei_1_1ProcessorFactory.html</filename>
    <member kind="function" static="yes">
      <type>const QStringList</type>
      <name>available</name>
      <anchor>e0</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function" static="yes">
      <type>const bool</type>
      <name>available</name>
      <anchor>e1</anchor>
      <arglist>(const QString &amp;type)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>Processor *</type>
      <name>create</name>
      <anchor>e2</anchor>
      <arglist>(const QString &amp;type)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>QString</type>
      <name>version</name>
      <anchor>e3</anchor>
      <arglist>(const QString &amp;type)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>const int</type>
      <name>versionId</name>
      <anchor>e4</anchor>
      <arglist>(const QString &amp;type)</arglist>
    </member>
  </compound>
  <compound kind="class">
    <name>Geddei::ProcessorGroup</name>
    <filename>classGeddei_1_1ProcessorGroup.html</filename>
    <member kind="function">
      <type>const bool</type>
      <name>confirmTypes</name>
      <anchor>a0</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function">
      <type>const uint</type>
      <name>count</name>
      <anchor>a1</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>deleteAll</name>
      <anchor>a2</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>disconnectAll</name>
      <anchor>a3</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>DomProcessor &amp;</type>
      <name>dom</name>
      <anchor>a4</anchor>
      <arglist>(const QString &amp;name)</arglist>
    </member>
    <member kind="function">
      <type>const bool</type>
      <name>exists</name>
      <anchor>a5</anchor>
      <arglist>(const QString &amp;name)</arglist>
    </member>
    <member kind="function">
      <type>Processor &amp;</type>
      <name>get</name>
      <anchor>a6</anchor>
      <arglist>(const QString &amp;name)</arglist>
    </member>
    <member kind="function">
      <type>const bool</type>
      <name>go</name>
      <anchor>a7</anchor>
      <arglist>(const bool waitUntilGoing=true) const</arglist>
    </member>
    <member kind="function">
      <type>Processor &amp;</type>
      <name>operator[]</name>
      <anchor>a8</anchor>
      <arglist>(const QString &amp;name)</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>ProcessorGroup</name>
      <anchor>a9</anchor>
      <arglist>(const bool adopt=false)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>reset</name>
      <anchor>a10</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>stop</name>
      <anchor>a11</anchor>
      <arglist>(const bool resetToo=true) const</arglist>
    </member>
    <member kind="function">
      <type>const Processor::ErrorType</type>
      <name>waitUntilGoing</name>
      <anchor>a12</anchor>
      <arglist>(Processor **errorProc=0, int *errorData=0) const</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>~ProcessorGroup</name>
      <anchor>a13</anchor>
      <arglist>()</arglist>
    </member>
  </compound>
  <compound kind="class">
    <name>Geddei::ProcessorPort</name>
    <filename>classGeddei_1_1ProcessorPort.html</filename>
    <member kind="function">
      <type>void</type>
      <name>connect</name>
      <anchor>a0</anchor>
      <arglist>(const ProcessorPort &amp;input) const</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>disconnect</name>
      <anchor>a1</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>operator--</name>
      <anchor>a2</anchor>
      <arglist>(int) const</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>operator&gt;&gt;=</name>
      <anchor>a3</anchor>
      <arglist>(const ProcessorPort &amp;input) const</arglist>
    </member>
    <member kind="function">
      <type>ProcessorPort &amp;</type>
      <name>setSize</name>
      <anchor>a4</anchor>
      <arglist>(const uint bufferSize)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>share</name>
      <anchor>a5</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>split</name>
      <anchor>a6</anchor>
      <arglist>() const</arglist>
    </member>
  </compound>
  <compound kind="class">
    <name>Geddei::Properties</name>
    <filename>classGeddei_1_1Properties.html</filename>
    <member kind="function">
      <type>const QVariant &amp;</type>
      <name>get</name>
      <anchor>a0</anchor>
      <arglist>(const QString &amp;key) const</arglist>
    </member>
    <member kind="function">
      <type>const QStringList</type>
      <name>keys</name>
      <anchor>a1</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function">
      <type>Properties &amp;</type>
      <name>operator()</name>
      <anchor>a2</anchor>
      <arglist>(const QString &amp;key, const QVariant &amp;value)</arglist>
    </member>
    <member kind="function">
      <type>QVariant &amp;</type>
      <name>operator[]</name>
      <anchor>a3</anchor>
      <arglist>(const QString &amp;key)</arglist>
    </member>
    <member kind="function">
      <type>const QVariant &amp;</type>
      <name>operator[]</name>
      <anchor>a4</anchor>
      <arglist>(const QString &amp;key) const</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>Properties</name>
      <anchor>a5</anchor>
      <arglist>(const PropertiesInfo &amp;info)</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>Properties</name>
      <anchor>a6</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>Properties</name>
      <anchor>a7</anchor>
      <arglist>(const QString &amp;key, const QVariant &amp;value)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>set</name>
      <anchor>a8</anchor>
      <arglist>(const Properties &amp;pairs)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>set</name>
      <anchor>a9</anchor>
      <arglist>(const QString &amp;key, const QVariant value)</arglist>
    </member>
    <member kind="function">
      <type>const uint</type>
      <name>size</name>
      <anchor>a10</anchor>
      <arglist>() const</arglist>
    </member>
  </compound>
  <compound kind="class">
    <name>Geddei::PropertiesInfo</name>
    <filename>classGeddei_1_1PropertiesInfo.html</filename>
    <base protection="private">Geddei::Properties</base>
    <member kind="function">
      <type>const QVariant &amp;</type>
      <name>defaultValue</name>
      <anchor>a0</anchor>
      <arglist>(const QString &amp;key) const</arglist>
    </member>
    <member kind="function">
      <type>const QString</type>
      <name>description</name>
      <anchor>a1</anchor>
      <arglist>(const QString &amp;key) const</arglist>
    </member>
    <member kind="function">
      <type>const QStringList</type>
      <name>keys</name>
      <anchor>a2</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function">
      <type>PropertiesInfo &amp;</type>
      <name>operator()</name>
      <anchor>a3</anchor>
      <arglist>(const QString &amp;key, const QVariant defaultValue, const QString &amp;description)</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>PropertiesInfo</name>
      <anchor>a4</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>PropertiesInfo</name>
      <anchor>a5</anchor>
      <arglist>(const QString &amp;key, const QVariant defaultValue, const QString &amp;description)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>set</name>
      <anchor>a6</anchor>
      <arglist>(const QString &amp;key, const QVariant defaultValue, const QString &amp;description)</arglist>
    </member>
    <member kind="function">
      <type>const uint</type>
      <name>size</name>
      <anchor>a7</anchor>
      <arglist>() const</arglist>
    </member>
  </compound>
  <compound kind="class">
    <name>Geddei::SignalType</name>
    <filename>classGeddei_1_1SignalType.html</filename>
    <member kind="function">
      <type>const uint</type>
      <name>elementsFromSamples</name>
      <anchor>a0</anchor>
      <arglist>(const uint samples) const</arglist>
    </member>
    <member kind="function">
      <type>const uint</type>
      <name>elementsFromSeconds</name>
      <anchor>a1</anchor>
      <arglist>(const float seconds) const</arglist>
    </member>
    <member kind="function">
      <type>const float</type>
      <name>frequency</name>
      <anchor>a2</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function">
      <type>const uint</type>
      <name>samples</name>
      <anchor>a3</anchor>
      <arglist>(const uint elements) const</arglist>
    </member>
    <member kind="function">
      <type>const uint</type>
      <name>scope</name>
      <anchor>a4</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function">
      <type>const float</type>
      <name>seconds</name>
      <anchor>a5</anchor>
      <arglist>(const uint elements) const</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>setFrequency</name>
      <anchor>a6</anchor>
      <arglist>(const float frequency)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>setScope</name>
      <anchor>a7</anchor>
      <arglist>(const uint scope)</arglist>
    </member>
    <member kind="function" protection="protected">
      <type></type>
      <name>SignalType</name>
      <anchor>b0</anchor>
      <arglist>(const uint scope=1, const float frequency=0)</arglist>
    </member>
    <member kind="function" protection="protected" virtualness="pure">
      <type>virtual SignalType *</type>
      <name>copyBE</name>
      <anchor>z23_0</anchor>
      <arglist>() const =0</arglist>
    </member>
    <member kind="function" protection="protected" virtualness="virtual">
      <type>virtual void</type>
      <name>deserialise</name>
      <anchor>z23_1</anchor>
      <arglist>(QSocketSession &amp;source)</arglist>
    </member>
    <member kind="function" protection="protected" virtualness="pure">
      <type>virtual const uint</type>
      <name>id</name>
      <anchor>z23_2</anchor>
      <arglist>() const =0</arglist>
    </member>
    <member kind="function" protection="protected" virtualness="virtual">
      <type>virtual const bool</type>
      <name>sameAsBE</name>
      <anchor>z23_3</anchor>
      <arglist>(const SignalType *comp) const</arglist>
    </member>
    <member kind="function" protection="protected" virtualness="virtual">
      <type>virtual void</type>
      <name>serialise</name>
      <anchor>z23_4</anchor>
      <arglist>(QSocketSession &amp;sink) const</arglist>
    </member>
    <member kind="variable" protection="protected">
      <type>float</type>
      <name>theFrequency</name>
      <anchor>p0</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable" protection="protected">
      <type>uint</type>
      <name>theScope</name>
      <anchor>p1</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="class">
    <name>Geddei::SignalTypeRef</name>
    <filename>classGeddei_1_1SignalTypeRef.html</filename>
    <member kind="function">
      <type>T &amp;</type>
      <name>asA</name>
      <anchor>a0</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>const T &amp;</type>
      <name>asA</name>
      <anchor>a1</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function">
      <type>const float</type>
      <name>frequency</name>
      <anchor>a2</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function">
      <type>const bool</type>
      <name>isA</name>
      <anchor>a3</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function">
      <type>SignalTypeRef &amp;</type>
      <name>operator=</name>
      <anchor>a4</anchor>
      <arglist>(const SignalTypeRef &amp;p)</arglist>
    </member>
    <member kind="function">
      <type>SignalTypeRef &amp;</type>
      <name>operator=</name>
      <anchor>a5</anchor>
      <arglist>(const SignalType &amp;p)</arglist>
    </member>
    <member kind="function">
      <type>const bool</type>
      <name>operator==</name>
      <anchor>a6</anchor>
      <arglist>(const SignalTypeRef &amp;p)</arglist>
    </member>
    <member kind="function">
      <type>const bool</type>
      <name>operator==</name>
      <anchor>a7</anchor>
      <arglist>(const SignalType &amp;p)</arglist>
    </member>
    <member kind="function">
      <type>const uint</type>
      <name>scope</name>
      <anchor>a8</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>SignalTypeRef</name>
      <anchor>a9</anchor>
      <arglist>(const SignalTypeRef &amp;src)</arglist>
    </member>
  </compound>
  <compound kind="class">
    <name>Geddei::SignalTypeRefs</name>
    <filename>classGeddei_1_1SignalTypeRefs.html</filename>
    <member kind="function">
      <type>const bool</type>
      <name>allSame</name>
      <anchor>a0</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function">
      <type>SignalTypeRefs &amp;</type>
      <name>operator=</name>
      <anchor>a1</anchor>
      <arglist>(const SignalType &amp;src)</arglist>
    </member>
    <member kind="function">
      <type>SignalTypeRefs &amp;</type>
      <name>operator=</name>
      <anchor>a2</anchor>
      <arglist>(const SignalTypeRef &amp;src)</arglist>
    </member>
    <member kind="function">
      <type>SignalTypeRefs &amp;</type>
      <name>operator=</name>
      <anchor>a3</anchor>
      <arglist>(const SignalTypeRefs &amp;src)</arglist>
    </member>
    <member kind="function">
      <type>SignalTypeRef</type>
      <name>operator[]</name>
      <anchor>a4</anchor>
      <arglist>(const uint i)</arglist>
    </member>
    <member kind="function">
      <type>const SignalTypeRef</type>
      <name>operator[]</name>
      <anchor>a5</anchor>
      <arglist>(const uint i) const</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>~SignalTypeRefs</name>
      <anchor>a6</anchor>
      <arglist>()</arglist>
    </member>
  </compound>
  <compound kind="class">
    <name>Geddei::SubFactoryCreator</name>
    <filename>classGeddei_1_1SubFactoryCreator.html</filename>
    <base>Geddei::MultiProcessorCreator</base>
    <member kind="function">
      <type></type>
      <name>SubFactoryCreator</name>
      <anchor>a0</anchor>
      <arglist>(const QString &amp;type)</arglist>
    </member>
  </compound>
  <compound kind="class">
    <name>Geddei::SubProcessor</name>
    <filename>classGeddei_1_1SubProcessor.html</filename>
    <member kind="function" virtualness="virtual">
      <type>virtual</type>
      <name>~SubProcessor</name>
      <anchor>a0</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function" protection="protected" virtualness="pure">
      <type>virtual void</type>
      <name>initFromProperties</name>
      <anchor>b0</anchor>
      <arglist>(const Properties &amp;properties)=0</arglist>
    </member>
    <member kind="function" protection="protected">
      <type>const uint</type>
      <name>multiplicity</name>
      <anchor>b1</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function" protection="protected" virtualness="virtual">
      <type>virtual void</type>
      <name>paintProcessor</name>
      <anchor>b2</anchor>
      <arglist>(QPainter &amp;p)</arglist>
    </member>
    <member kind="function" protection="protected" virtualness="virtual">
      <type>virtual void</type>
      <name>processChunk</name>
      <anchor>b3</anchor>
      <arglist>(const BufferDatas &amp;in, BufferDatas &amp;out) const</arglist>
    </member>
    <member kind="function" protection="protected" virtualness="virtual">
      <type>virtual void</type>
      <name>processChunks</name>
      <anchor>b4</anchor>
      <arglist>(const BufferDatas &amp;in, BufferDatas &amp;out, const uint chunks) const</arglist>
    </member>
    <member kind="function" protection="protected">
      <type>void</type>
      <name>setupIO</name>
      <anchor>b5</anchor>
      <arglist>(const uint numInputs=1, const uint numOutputs=1, const uint samplesIn=1, const uint samplesStep=1, const uint samplesOut=1)</arglist>
    </member>
    <member kind="function" protection="protected">
      <type>void</type>
      <name>setupVisual</name>
      <anchor>b6</anchor>
      <arglist>(const uint width=50, const uint height=30, const uint redrawPeriod=0)</arglist>
    </member>
    <member kind="function" protection="protected" virtualness="virtual">
      <type>virtual PropertiesInfo</type>
      <name>specifyProperties</name>
      <anchor>b7</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function" protection="protected">
      <type></type>
      <name>SubProcessor</name>
      <anchor>b8</anchor>
      <arglist>(const QString &amp;type, const MultiplicityType &amp;multi=NotMulti)</arglist>
    </member>
    <member kind="function" protection="protected" virtualness="pure">
      <type>virtual const bool</type>
      <name>verifyAndSpecifyTypes</name>
      <anchor>b9</anchor>
      <arglist>(const SignalTypeRefs &amp;inTypes, SignalTypeRefs &amp;outTypes)=0</arglist>
    </member>
  </compound>
  <compound kind="class">
    <name>Geddei::SubProcessorFactory</name>
    <filename>classGeddei_1_1SubProcessorFactory.html</filename>
    <member kind="function" static="yes">
      <type>const QStringList</type>
      <name>available</name>
      <anchor>e0</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function" static="yes">
      <type>const bool</type>
      <name>available</name>
      <anchor>e1</anchor>
      <arglist>(const QString &amp;type)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>SubProcessor *</type>
      <name>create</name>
      <anchor>e2</anchor>
      <arglist>(const QString &amp;type)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>DomProcessor *</type>
      <name>createDom</name>
      <anchor>e3</anchor>
      <arglist>(const QString &amp;type)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>QString</type>
      <name>version</name>
      <anchor>e4</anchor>
      <arglist>(const QString &amp;type)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>const int</type>
      <name>versionId</name>
      <anchor>e5</anchor>
      <arglist>(const QString &amp;type)</arglist>
    </member>
  </compound>
  <compound kind="class">
    <name>Geddei::xLConnection</name>
    <filename>classGeddei_1_1xLConnection.html</filename>
    <member kind="function" virtualness="pure">
      <type>virtual const uint</type>
      <name>capacity</name>
      <anchor>a0</anchor>
      <arglist>() const =0</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual const float</type>
      <name>filled</name>
      <anchor>a1</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function">
      <type>const BufferData</type>
      <name>peekSample</name>
      <anchor>a2</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>const BufferData</type>
      <name>peekSamples</name>
      <anchor>a3</anchor>
      <arglist>(const uint samples=0, const bool allowZero=false)</arglist>
    </member>
    <member kind="function">
      <type>const BufferData</type>
      <name>peekSecond</name>
      <anchor>a4</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>const BufferData</type>
      <name>peekSeconds</name>
      <anchor>a5</anchor>
      <arglist>(const float seconds)</arglist>
    </member>
    <member kind="function">
      <type>const BufferData</type>
      <name>readSample</name>
      <anchor>a6</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>const BufferData</type>
      <name>readSamples</name>
      <anchor>a7</anchor>
      <arglist>(const uint samples=0, const bool allowZero=false)</arglist>
    </member>
    <member kind="function">
      <type>const BufferData</type>
      <name>readSecond</name>
      <anchor>a8</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>const BufferData</type>
      <name>readSeconds</name>
      <anchor>a9</anchor>
      <arglist>(const float seconds)</arglist>
    </member>
    <member kind="function">
      <type>const uint</type>
      <name>samplesReady</name>
      <anchor>a10</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual const SignalTypeRef</type>
      <name>type</name>
      <anchor>a11</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>waitForSamples</name>
      <anchor>a12</anchor>
      <arglist>(const uint samples=1) const</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual</type>
      <name>~xLConnection</name>
      <anchor>a13</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function" protection="protected" virtualness="pure">
      <type>virtual const uint</type>
      <name>elementsReady</name>
      <anchor>b0</anchor>
      <arglist>() const =0</arglist>
    </member>
    <member kind="function" protection="protected" virtualness="pure">
      <type>virtual void</type>
      <name>killReader</name>
      <anchor>b1</anchor>
      <arglist>()=0</arglist>
    </member>
    <member kind="function" protection="protected" virtualness="pure">
      <type>virtual BufferReader *</type>
      <name>newReader</name>
      <anchor>b2</anchor>
      <arglist>()=0</arglist>
    </member>
    <member kind="function" protection="protected" virtualness="pure">
      <type>virtual const BufferData</type>
      <name>peekElements</name>
      <anchor>b3</anchor>
      <arglist>(const uint elements)=0</arglist>
    </member>
    <member kind="function" protection="protected" virtualness="pure">
      <type>virtual const bool</type>
      <name>plungeSync</name>
      <anchor>b4</anchor>
      <arglist>(const uint samples) const =0</arglist>
    </member>
    <member kind="function" protection="protected" virtualness="pure">
      <type>virtual const BufferData</type>
      <name>readElements</name>
      <anchor>b5</anchor>
      <arglist>(const uint elements)=0</arglist>
    </member>
    <member kind="function" protection="protected" virtualness="pure">
      <type>virtual void</type>
      <name>reset</name>
      <anchor>b6</anchor>
      <arglist>()=0</arglist>
    </member>
    <member kind="function" protection="protected" virtualness="pure">
      <type>virtual void</type>
      <name>resurectReader</name>
      <anchor>b7</anchor>
      <arglist>()=0</arglist>
    </member>
    <member kind="function" protection="protected" virtualness="pure">
      <type>virtual void</type>
      <name>sinkStopped</name>
      <anchor>b8</anchor>
      <arglist>()=0</arglist>
    </member>
    <member kind="function" protection="protected" virtualness="pure">
      <type>virtual void</type>
      <name>sinkStopping</name>
      <anchor>b9</anchor>
      <arglist>()=0</arglist>
    </member>
    <member kind="function" protection="protected" virtualness="pure">
      <type>virtual void</type>
      <name>waitForElements</name>
      <anchor>b10</anchor>
      <arglist>(const uint elements) const =0</arglist>
    </member>
    <member kind="function" protection="protected">
      <type></type>
      <name>xLConnection</name>
      <anchor>b11</anchor>
      <arglist>(Sink *newSink, const uint newSinkIndex)</arglist>
    </member>
    <member kind="variable" protection="protected">
      <type>Sink *</type>
      <name>theSink</name>
      <anchor>p0</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable" protection="protected">
      <type>uint</type>
      <name>theSinkIndex</name>
      <anchor>p1</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="class">
    <name>rGeddei::AbstractDomProcessor</name>
    <filename>classrGeddei_1_1AbstractDomProcessor.html</filename>
    <base virtualness="virtual">rGeddei::AbstractProcessor</base>
    <member kind="function" virtualness="pure">
      <type>virtual const bool</type>
      <name>spawnWorker</name>
      <anchor>a0</anchor>
      <arglist>(RemoteSession &amp;session)=0</arglist>
    </member>
    <member kind="function" virtualness="pure">
      <type>virtual const bool</type>
      <name>spawnWorker</name>
      <anchor>a1</anchor>
      <arglist>(LocalSession &amp;session)=0</arglist>
    </member>
    <member kind="function" static="yes">
      <type>RemoteDomProcessor *</type>
      <name>create</name>
      <anchor>e0</anchor>
      <arglist>(RemoteSession &amp;session, const QString &amp;type)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>LocalDomProcessor *</type>
      <name>create</name>
      <anchor>e1</anchor>
      <arglist>(LocalSession &amp;session, const QString &amp;type)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>LocalDomProcessor *</type>
      <name>create</name>
      <anchor>e2</anchor>
      <arglist>(LocalSession &amp;session, SubProcessor *primary)</arglist>
    </member>
  </compound>
  <compound kind="class">
    <name>rGeddei::AbstractProcessor</name>
    <filename>classrGeddei_1_1AbstractProcessor.html</filename>
    <member kind="function" virtualness="pure">
      <type>virtual const bool</type>
      <name>connect</name>
      <anchor>a0</anchor>
      <arglist>(const uint sourceIndex, const LocalProcessor *sink, const uint sinkIndex, const uint bufferSize=1)=0</arglist>
    </member>
    <member kind="function" virtualness="pure">
      <type>virtual const bool</type>
      <name>connect</name>
      <anchor>a1</anchor>
      <arglist>(const uint sourceIndex, const RemoteProcessor *sink, const uint sinkIndex, const uint bufferSize=1)=0</arglist>
    </member>
    <member kind="function" virtualness="pure">
      <type>virtual void</type>
      <name>disconnect</name>
      <anchor>a2</anchor>
      <arglist>(const uint sourceIndex)=0</arglist>
    </member>
    <member kind="function" virtualness="pure">
      <type>virtual void</type>
      <name>disconnectAll</name>
      <anchor>a3</anchor>
      <arglist>()=0</arglist>
    </member>
    <member kind="function" virtualness="pure">
      <type>virtual const bool</type>
      <name>go</name>
      <anchor>a4</anchor>
      <arglist>()=0</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>init</name>
      <anchor>a5</anchor>
      <arglist>(const QString &amp;name=&quot;&quot;, const Properties &amp;p=Properties())</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>init</name>
      <anchor>a6</anchor>
      <arglist>(const QString &amp;name, AbstractProcessorGroup &amp;g, const Properties &amp;p=Properties())</arglist>
    </member>
    <member kind="function" virtualness="pure">
      <type>virtual const QString</type>
      <name>name</name>
      <anchor>a7</anchor>
      <arglist>() const =0</arglist>
    </member>
    <member kind="function">
      <type>AbstractProcessorPort</type>
      <name>operator[]</name>
      <anchor>a8</anchor>
      <arglist>(const uint port)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>setGroup</name>
      <anchor>a9</anchor>
      <arglist>(AbstractProcessorGroup &amp;g)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>setNoGroup</name>
      <anchor>a10</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function" virtualness="pure">
      <type>virtual void</type>
      <name>share</name>
      <anchor>a11</anchor>
      <arglist>(const uint sourceIndex)=0</arglist>
    </member>
    <member kind="function" virtualness="pure">
      <type>virtual void</type>
      <name>split</name>
      <anchor>a12</anchor>
      <arglist>(const uint sourceIndex)=0</arglist>
    </member>
    <member kind="function" virtualness="pure">
      <type>virtual void</type>
      <name>stop</name>
      <anchor>a13</anchor>
      <arglist>()=0</arglist>
    </member>
    <member kind="function" virtualness="pure">
      <type>virtual void</type>
      <name>waitUntilDone</name>
      <anchor>a14</anchor>
      <arglist>()=0</arglist>
    </member>
    <member kind="function" virtualness="pure">
      <type>virtual const Processor::ErrorType</type>
      <name>waitUntilGoing</name>
      <anchor>a15</anchor>
      <arglist>(int *errorData=0)=0</arglist>
    </member>
    <member kind="function" static="yes">
      <type>RemoteProcessor *</type>
      <name>create</name>
      <anchor>e0</anchor>
      <arglist>(RemoteSession &amp;session, const QString &amp;type)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>LocalProcessor *</type>
      <name>create</name>
      <anchor>e1</anchor>
      <arglist>(LocalSession &amp;session, const QString &amp;type)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>LocalProcessor *</type>
      <name>create</name>
      <anchor>e2</anchor>
      <arglist>(LocalSession &amp;session, Processor *processor)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>LocalProcessor *</type>
      <name>create</name>
      <anchor>e3</anchor>
      <arglist>(LocalSession &amp;session, Processor &amp;processor)</arglist>
    </member>
    <member kind="function" protection="protected" virtualness="pure">
      <type>virtual void</type>
      <name>doInit</name>
      <anchor>b1</anchor>
      <arglist>(const QString &amp;name, AbstractProcessorGroup *g, const Properties &amp;p)=0</arglist>
    </member>
  </compound>
  <compound kind="class">
    <name>rGeddei::AbstractProcessorGroup</name>
    <filename>classrGeddei_1_1AbstractProcessorGroup.html</filename>
    <member kind="function">
      <type></type>
      <name>AbstractProcessorGroup</name>
      <anchor>a0</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>deleteAll</name>
      <anchor>a1</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>disconnectAll</name>
      <anchor>a2</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>AbstractDomProcessor &amp;</type>
      <name>dom</name>
      <anchor>a3</anchor>
      <arglist>(const QString &amp;name)</arglist>
    </member>
    <member kind="function">
      <type>const bool</type>
      <name>exists</name>
      <anchor>a4</anchor>
      <arglist>(const QString &amp;name)</arglist>
    </member>
    <member kind="function">
      <type>AbstractProcessor &amp;</type>
      <name>get</name>
      <anchor>a5</anchor>
      <arglist>(const QString &amp;name)</arglist>
    </member>
    <member kind="function">
      <type>const bool</type>
      <name>go</name>
      <anchor>a6</anchor>
      <arglist>(const bool waitUntilGoing=true) const</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>init</name>
      <anchor>a7</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function">
      <type>AbstractProcessor &amp;</type>
      <name>operator[]</name>
      <anchor>a8</anchor>
      <arglist>(const QString &amp;name)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>stop</name>
      <anchor>a9</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function">
      <type>const Processor::ErrorType</type>
      <name>waitUntilGoing</name>
      <anchor>a10</anchor>
      <arglist>(AbstractProcessor **errorProc=0, int *errorData=0) const</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>~AbstractProcessorGroup</name>
      <anchor>a11</anchor>
      <arglist>()</arglist>
    </member>
  </compound>
  <compound kind="class">
    <name>rGeddei::AbstractProcessorPort</name>
    <filename>classrGeddei_1_1AbstractProcessorPort.html</filename>
    <member kind="function">
      <type>const bool</type>
      <name>connect</name>
      <anchor>a0</anchor>
      <arglist>(const AbstractProcessorPort &amp;input)</arglist>
    </member>
    <member kind="function">
      <type>const bool</type>
      <name>disconnect</name>
      <anchor>a1</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>const bool</type>
      <name>operator--</name>
      <anchor>a2</anchor>
      <arglist>(int)</arglist>
    </member>
    <member kind="function">
      <type>const bool</type>
      <name>operator&gt;&gt;=</name>
      <anchor>a3</anchor>
      <arglist>(const AbstractProcessorPort &amp;input)</arglist>
    </member>
    <member kind="function">
      <type>AbstractProcessorPort &amp;</type>
      <name>setSize</name>
      <anchor>a4</anchor>
      <arglist>(const uint bufferSize)</arglist>
    </member>
    <member kind="function">
      <type>const bool</type>
      <name>share</name>
      <anchor>a5</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>const bool</type>
      <name>split</name>
      <anchor>a6</anchor>
      <arglist>()</arglist>
    </member>
  </compound>
  <compound kind="class">
    <name>rGeddei::LocalDomProcessor</name>
    <filename>classrGeddei_1_1LocalDomProcessor.html</filename>
    <base virtualness="virtual">rGeddei::LocalProcessor</base>
    <base virtualness="virtual">rGeddei::AbstractDomProcessor</base>
    <member kind="function" virtualness="virtual">
      <type>virtual const bool</type>
      <name>spawnWorker</name>
      <anchor>a0</anchor>
      <arglist>(RemoteSession &amp;session)</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual const bool</type>
      <name>spawnWorker</name>
      <anchor>a1</anchor>
      <arglist>(LocalSession &amp;session)</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual</type>
      <name>~LocalDomProcessor</name>
      <anchor>a2</anchor>
      <arglist>()</arglist>
    </member>
  </compound>
  <compound kind="class">
    <name>rGeddei::LocalProcessor</name>
    <filename>classrGeddei_1_1LocalProcessor.html</filename>
    <base virtualness="virtual">rGeddei::AbstractProcessor</base>
    <member kind="function" virtualness="virtual">
      <type>virtual const bool</type>
      <name>connect</name>
      <anchor>a0</anchor>
      <arglist>(const uint sourceIndex, const LocalProcessor *sink, const uint sinkIndex, const uint bufferSize=1)</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual const bool</type>
      <name>connect</name>
      <anchor>a1</anchor>
      <arglist>(const uint sourceIndex, const RemoteProcessor *sink, const uint sinkIndex, const uint bufferSize=1)</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual void</type>
      <name>disconnect</name>
      <anchor>a2</anchor>
      <arglist>(const uint sourceIndex)</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual void</type>
      <name>disconnectAll</name>
      <anchor>a3</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual const bool</type>
      <name>go</name>
      <anchor>a4</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual const QString</type>
      <name>name</name>
      <anchor>a5</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual void</type>
      <name>share</name>
      <anchor>a6</anchor>
      <arglist>(const uint sourceIndex)</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual void</type>
      <name>split</name>
      <anchor>a7</anchor>
      <arglist>(const uint sourceIndex)</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual void</type>
      <name>stop</name>
      <anchor>a8</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual void</type>
      <name>waitUntilDone</name>
      <anchor>a9</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual const Processor::ErrorType</type>
      <name>waitUntilGoing</name>
      <anchor>a10</anchor>
      <arglist>(int *errorData=0)</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual</type>
      <name>~LocalProcessor</name>
      <anchor>a11</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="variable" protection="protected">
      <type>bool</type>
      <name>theAdopted</name>
      <anchor>p0</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable" protection="protected">
      <type>Processor *</type>
      <name>theProcessor</name>
      <anchor>p1</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable" protection="protected">
      <type>LocalSession *</type>
      <name>theSession</name>
      <anchor>p2</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="class">
    <name>rGeddei::LocalSession</name>
    <filename>classrGeddei_1_1LocalSession.html</filename>
    <base protection="private">Geddei::ProcessorGroup</base>
    <member kind="function">
      <type></type>
      <name>LocalSession</name>
      <anchor>a0</anchor>
      <arglist>(const QString &amp;host, uint port=GEDDEI_PORT-1)</arglist>
    </member>
  </compound>
  <compound kind="class">
    <name>rGeddei::RemoteDomProcessor</name>
    <filename>classrGeddei_1_1RemoteDomProcessor.html</filename>
    <base virtualness="virtual">rGeddei::RemoteProcessor</base>
    <base virtualness="virtual">rGeddei::AbstractDomProcessor</base>
    <member kind="function" virtualness="virtual">
      <type>virtual const bool</type>
      <name>spawnWorker</name>
      <anchor>a0</anchor>
      <arglist>(RemoteSession &amp;session)</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual const bool</type>
      <name>spawnWorker</name>
      <anchor>a1</anchor>
      <arglist>(LocalSession &amp;session)</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual</type>
      <name>~RemoteDomProcessor</name>
      <anchor>a2</anchor>
      <arglist>()</arglist>
    </member>
  </compound>
  <compound kind="class">
    <name>rGeddei::RemoteProcessor</name>
    <filename>classrGeddei_1_1RemoteProcessor.html</filename>
    <base virtualness="virtual">rGeddei::AbstractProcessor</base>
    <member kind="function" virtualness="virtual">
      <type>virtual const bool</type>
      <name>connect</name>
      <anchor>a0</anchor>
      <arglist>(const uint sourceIndex, const LocalProcessor *sink, const uint sinkIndex, const uint bufferSize=1)</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual const bool</type>
      <name>connect</name>
      <anchor>a1</anchor>
      <arglist>(const uint sourceIndex, const RemoteProcessor *sink, const uint sinkIndex, const uint bufferSize=1)</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual void</type>
      <name>disconnect</name>
      <anchor>a2</anchor>
      <arglist>(const uint sourceIndex)</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual void</type>
      <name>disconnectAll</name>
      <anchor>a3</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual const bool</type>
      <name>go</name>
      <anchor>a4</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual const QString</type>
      <name>name</name>
      <anchor>a5</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual void</type>
      <name>share</name>
      <anchor>a6</anchor>
      <arglist>(const uint sourceIndex)</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual void</type>
      <name>split</name>
      <anchor>a7</anchor>
      <arglist>(const uint sourceIndex)</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual void</type>
      <name>stop</name>
      <anchor>a8</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual void</type>
      <name>waitUntilDone</name>
      <anchor>a9</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual const Processor::ErrorType</type>
      <name>waitUntilGoing</name>
      <anchor>a10</anchor>
      <arglist>(int *errorData=0)</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual</type>
      <name>~RemoteProcessor</name>
      <anchor>a11</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function" protection="protected">
      <type></type>
      <name>RemoteProcessor</name>
      <anchor>b0</anchor>
      <arglist>(RemoteSession &amp;session)</arglist>
    </member>
  </compound>
  <compound kind="class">
    <name>rGeddei::RemoteSession</name>
    <filename>classrGeddei_1_1RemoteSession.html</filename>
    <member kind="function">
      <type>const bool</type>
      <name>available</name>
      <anchor>a0</anchor>
      <arglist>(const QString &amp;type)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>deleteDomProcessor</name>
      <anchor>a1</anchor>
      <arglist>(const QString &amp;name)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>deleteProcessor</name>
      <anchor>a2</anchor>
      <arglist>(const QString &amp;name)</arglist>
    </member>
    <member kind="function">
      <type>const bool</type>
      <name>domProcessorCreateAndAdd</name>
      <anchor>a3</anchor>
      <arglist>(const QString &amp;name, const QString &amp;host, const uint hostKey)</arglist>
    </member>
    <member kind="function">
      <type>const bool</type>
      <name>domProcessorCreateAndAdd</name>
      <anchor>a4</anchor>
      <arglist>(const QString &amp;name)</arglist>
    </member>
    <member kind="function">
      <type>const bool</type>
      <name>isValid</name>
      <anchor>a5</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>keepAlive</name>
      <anchor>a6</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>const QString</type>
      <name>makeUniqueProcessorName</name>
      <anchor>a7</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>const bool</type>
      <name>newDomProcessor</name>
      <anchor>a8</anchor>
      <arglist>(const QString &amp;subType, const QString &amp;name)</arglist>
    </member>
    <member kind="function">
      <type>const bool</type>
      <name>newProcessor</name>
      <anchor>a9</anchor>
      <arglist>(const QString &amp;type, const QString &amp;name)</arglist>
    </member>
    <member kind="function">
      <type>const bool</type>
      <name>processorConnect</name>
      <anchor>a10</anchor>
      <arglist>(const QString &amp;name, const uint bufferSize, const uint output, const QString &amp;destHost, const uint destKey, const QString &amp;destName, const uint destInput)</arglist>
    </member>
    <member kind="function">
      <type>const bool</type>
      <name>processorConnect</name>
      <anchor>a11</anchor>
      <arglist>(const QString &amp;name, const uint bufferSize, const uint output, const QString &amp;destName, const uint destInput)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>processorDisconnect</name>
      <anchor>a12</anchor>
      <arglist>(const QString &amp;name, const uint output)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>processorDisconnectAll</name>
      <anchor>a13</anchor>
      <arglist>(const QString &amp;name)</arglist>
    </member>
    <member kind="function">
      <type>const bool</type>
      <name>processorGo</name>
      <anchor>a14</anchor>
      <arglist>(const QString &amp;name)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>processorInit</name>
      <anchor>a15</anchor>
      <arglist>(const QString &amp;name, const Properties &amp;p, const QString &amp;newName)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>processorShare</name>
      <anchor>a16</anchor>
      <arglist>(const QString &amp;name, const uint output)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>processorSplit</name>
      <anchor>a17</anchor>
      <arglist>(const QString &amp;name, const uint output)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>processorStop</name>
      <anchor>a18</anchor>
      <arglist>(const QString &amp;name)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>processorWaitUntilDone</name>
      <anchor>a19</anchor>
      <arglist>(const QString &amp;name)</arglist>
    </member>
    <member kind="function">
      <type>const int</type>
      <name>processorWaitUntilGoing</name>
      <anchor>a20</anchor>
      <arglist>(const QString &amp;name, int &amp;errorData)</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>RemoteSession</name>
      <anchor>a21</anchor>
      <arglist>(const QString &amp;host, const uint port=RGEDDEI_PORT)</arglist>
    </member>
    <member kind="function">
      <type>const bool</type>
      <name>subAvailable</name>
      <anchor>a22</anchor>
      <arglist>(const QString &amp;type)</arglist>
    </member>
    <member kind="function">
      <type>const int</type>
      <name>subVersion</name>
      <anchor>a23</anchor>
      <arglist>(const QString &amp;type)</arglist>
    </member>
    <member kind="function">
      <type>const bool</type>
      <name>typeAvailable</name>
      <anchor>a24</anchor>
      <arglist>(const QString &amp;type)</arglist>
    </member>
    <member kind="function">
      <type>const bool</type>
      <name>typeSubAvailable</name>
      <anchor>a25</anchor>
      <arglist>(const QString &amp;type)</arglist>
    </member>
    <member kind="function">
      <type>const int</type>
      <name>typeSubVersion</name>
      <anchor>a26</anchor>
      <arglist>(const QString &amp;type)</arglist>
    </member>
    <member kind="function">
      <type>const int</type>
      <name>typeVersion</name>
      <anchor>a27</anchor>
      <arglist>(const QString &amp;type)</arglist>
    </member>
    <member kind="function">
      <type>const int</type>
      <name>version</name>
      <anchor>a28</anchor>
      <arglist>(const QString &amp;type)</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>~RemoteSession</name>
      <anchor>a29</anchor>
      <arglist>()</arglist>
    </member>
  </compound>
  <compound kind="class">
    <name>SignalTypes::Matrix</name>
    <filename>classSignalTypes_1_1Matrix.html</filename>
    <base>Geddei::SignalType</base>
    <member kind="function">
      <type>const uint</type>
      <name>height</name>
      <anchor>a0</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>Matrix</name>
      <anchor>a1</anchor>
      <arglist>(const uint width=1, const uint height=1, const float frequency=0, const float pitchWidth=0, const float pitchHeight=0)</arglist>
    </member>
    <member kind="function">
      <type>const float</type>
      <name>pitchHeight</name>
      <anchor>a2</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function">
      <type>const float</type>
      <name>pitchWidth</name>
      <anchor>a3</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function">
      <type>const uint</type>
      <name>width</name>
      <anchor>a4</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="variable" protection="protected">
      <type>uint</type>
      <name>theHeight</name>
      <anchor>p0</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable" protection="protected">
      <type>float</type>
      <name>thePitchHeight</name>
      <anchor>p1</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable" protection="protected">
      <type>float</type>
      <name>thePitchWidth</name>
      <anchor>p2</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable" protection="protected">
      <type>uint</type>
      <name>theWidth</name>
      <anchor>p3</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="class">
    <name>SignalTypes::Spectrum</name>
    <filename>classSignalTypes_1_1Spectrum.html</filename>
    <base>Geddei::SignalType</base>
    <member kind="function">
      <type>const float</type>
      <name>bandFrequency</name>
      <anchor>a0</anchor>
      <arglist>(uint band) const</arglist>
    </member>
    <member kind="function">
      <type>const float</type>
      <name>nyquist</name>
      <anchor>a1</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function">
      <type>const uint</type>
      <name>size</name>
      <anchor>a2</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>Spectrum</name>
      <anchor>a3</anchor>
      <arglist>(const uint size=1, const float frequency=0, const float step=1)</arglist>
    </member>
    <member kind="function">
      <type>const float</type>
      <name>step</name>
      <anchor>a4</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="variable" protection="protected">
      <type>float</type>
      <name>theStep</name>
      <anchor>p0</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="class">
    <name>SignalTypes::SquareMatrix</name>
    <filename>classSignalTypes_1_1SquareMatrix.html</filename>
    <base>SignalTypes::Matrix</base>
    <member kind="function">
      <type>const float</type>
      <name>pitch</name>
      <anchor>a0</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function">
      <type>const uint</type>
      <name>size</name>
      <anchor>a1</anchor>
      <arglist>() const</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>SquareMatrix</name>
      <anchor>a2</anchor>
      <arglist>(const uint size=1, const float frequency=0., const float pitch=0.)</arglist>
    </member>
  </compound>
  <compound kind="class">
    <name>SignalTypes::Value</name>
    <filename>classSignalTypes_1_1Value.html</filename>
    <base>Geddei::SignalType</base>
    <member kind="function">
      <type></type>
      <name>Value</name>
      <anchor>a0</anchor>
      <arglist>(const float frequency=0)</arglist>
    </member>
  </compound>
  <compound kind="class">
    <name>SignalTypes::Wave</name>
    <filename>classSignalTypes_1_1Wave.html</filename>
    <base>Geddei::SignalType</base>
    <member kind="function">
      <type></type>
      <name>Wave</name>
      <anchor>a0</anchor>
      <arglist>(const float frequency=0)</arglist>
    </member>
  </compound>
</tagfile>
