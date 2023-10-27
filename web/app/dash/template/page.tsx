import { ArrowDownOnSquareIcon } from "@heroicons/react/20/solid";

const CodeBlock = () => {
  return (
    <pre className="text-white p-4 select-none">
      <code>
        <span className="text-blue-400">def </span>place_market_order<span className="text-red-400">(</span><span className="text-blue-400">side</span><span className="text-red-400">: </span><span className="text-red-400">str</span><span className="text-red-400">, </span><span className="text-blue-400">ticker</span><span className="text-red-400">: </span><span className="text-red-400">str</span><span className="text-red-400">, </span><span className="text-blue-400">quantity</span><span className="text-red-400">: </span><span className="text-red-400">float</span><span className="text-red-400">, </span><span className="text-blue-400">price</span><span className="text-red-400">: </span><span className="text-red-400">float</span><span className="text-red-400">)</span> -&gt; <span className="text-red-400">None</span>:<br />
        <span className="text-green-400">    &quot;&quot;&quot;Place an order - DO NOT MODIFY&quot;&quot;&quot;</span><br />
        <span>    </span><br />
        <span className="text-blue-400">class</span> Strategy:<br />
        <span className="text-green-400">    &quot;&quot;&quot;Template for a strategy.&quot;&quot;&quot;</span><br />
        <br />
        <span className="text-blue-400">    def</span> __init__<span className="text-red-400">(</span><span className="text-blue-400">self</span><span className="text-red-400">)</span> -&gt; <span className="text-red-400">None</span>:<br />
        <span className="text-green-400">        &quot;&quot;&quot;Your initialization code goes here.&quot;&quot;&quot;</span><br />
        <br />
        <span className="text-blue-400">    def</span> on_trade_update<span className="text-red-400">(</span><span className="text-blue-400">self</span><span className="text-red-400">, </span><span className="text-blue-400">ticker</span><span className="text-red-400">: </span><span className="text-red-400">str</span><span className="text-red-400">, </span><span className="text-blue-400">side</span><span className="text-red-400">: </span><span className="text-red-400">str</span><span className="text-red-400">, </span><span className="text-blue-400">price</span><span className="text-red-400">: </span><span className="text-red-400">float</span><span className="text-red-400">, </span><span className="text-blue-400">quantity</span><span className="text-red-400">: </span><span className="text-red-400">float</span><span className="text-red-400">)</span> -&gt; <span className="text-red-400">None</span>:<br />
        <span className="text-white">        print</span><span className="text-red-400">(</span><span className="text-green-400">f&quot;Python Trade update: </span><span className="text-red-400">&#123;</span><span className="text-white">ticker</span><span className="text-red-400">&#125;</span><span className="text-green-400"> </span><span className="text-red-400">&#123;</span><span className="text-white">side</span><span className="text-red-400">&#125;</span><span className="text-green-400"> </span><span className="text-red-400">&#123;</span><span className="text-white">price</span><span className="text-red-400">&#125;</span><span className="text-green-400"> </span><span className="text-red-400">&#123;</span><span className="text-white">quantity</span><span className="text-red-400">&#125;</span><span className="text-green-400">&quot;</span><span className="text-red-400">)</span><br />
        <br />
        <span className="text-blue-400">    def</span> on_orderbook_update<span className="text-red-400">(</span><span className="text-blue-400">self</span><span className="text-red-400">, </span><span className="text-blue-400">ticker</span><span className="text-red-400">: </span><span className="text-red-400">str</span><span className="text-red-400">, </span><span className="text-blue-400">side</span><span className="text-red-400">: </span><span className="text-red-400">str</span><span className="text-red-400">, </span><span className="text-blue-400">price</span><span className="text-red-400">: </span><span className="text-red-400">float</span><span className="text-red-400">, </span><span className="text-blue-400">quantity</span><span className="text-red-400">: </span><span className="text-red-400">bool</span><span className="text-red-400">)</span> -&gt; <span className="text-red-400">None</span>:<br />
        <span className="text-white">        print</span><span className="text-red-400">(</span><span className="text-green-400">f&quot;Python Orderbook update: </span><span className="text-red-400">&#123;</span><span className="text-white">ticker</span><span className="text-red-400">&#125;</span><span className="text-green-400"> </span><span className="text-red-400">&#123;</span><span className="text-white">side</span><span className="text-red-400">&#125;</span><span className="text-green-400"> </span><span className="text-red-400">&#123;</span><span className="text-white">price</span><span className="text-red-400">&#125;</span><span className="text-green-400"> </span><span className="text-red-400">&#123;</span><span className="text-white">quantity</span><span className="text-red-400">&#125;</span><span className="text-green-400">&quot;</span><span className="text-red-400">)</span><br />
        <br />
        <span className="text-blue-400">    def</span> on_account_update<span className="text-red-400">(</span><span className="text-blue-400">self</span><span className="text-red-400">, </span><span className="text-blue-400">ticker</span><span className="text-red-400">: </span><span className="text-red-400">str</span><span className="text-red-400">, </span><span className="text-blue-400">side</span><span className="text-red-400">: </span><span className="text-red-400">str</span><span className="text-red-400">, </span><span className="text-blue-400">price</span><span className="text-red-400">: </span><span className="text-red-400">float</span><span className="text-red-400">, </span><span className="text-blue-400">quantity</span><span className="text-red-400">: </span><span className="text-red-400">float</span><span className="text-red-400">, </span><span className="text-blue-400">capital_remaining</span><span className="text-red-400">: </span><span className="text-red-400">float</span><span className="text-red-400">)</span> -&gt; <span className="text-red-400">None</span>:<br />
        <span className="text-white">        print</span><span className="text-red-400">(</span><span className="text-green-400">f&quot;Python Account update: </span><span className="text-red-400">&#123;</span><span className="text-white">ticker</span><span className="text-red-400">&#125;</span><span className="text-green-400"> </span><span className="text-red-400">&#123;</span><span className="text-white">side</span><span className="text-red-400">&#125;</span><span className="text-green-400"> </span><span className="text-red-400">&#123;</span><span className="text-white">price</span><span className="text-red-400">&#125;</span><span className="text-green-400"> </span><span className="text-red-400">&#123;</span><span className="text-white">quantity</span><span className="text-red-400">&#125;</span><span className="text-green-400"> </span><span className="text-red-400">&#123;</span><span className="text-white">capital_remaining</span><span className="text-red-400">&#125;</span><span className="text-green-400">&quot;</span><span className="text-red-400">)</span>
      </code>
    </pre>
  );
};

export default function Template() {
  return (
    <div className="flex flex-col items-center relative h-screen px-6 pt-8 sm:pt-16 md:pl-16 md:pr-16 ">
      <div className="mx-auto  md:mx-0 w-full">
        <div className="overflow-hidden rounded-tl-xl rounded-bl-xl rounded-br-xl rounded-tr-xl bg-gray-950">
          <div className="flex bg-gray-700/40 ring-1 ring-white/5">
            <div className="-mb-px flex text-sm font-medium leading-6 text-gray-400">
              <div className="border-b border-r border-b-white/20 border-r-white/10 bg-white/5 px-4 py-2 text-white">
                Template.py
              </div>
              <div className="border-r border-gray-600/10 px-4 py-2">
                
              </div>
            </div>
          </div>
          <div className="px-4 pb-10 pt-4 overflow-x-scroll">
            <CodeBlock />
          </div>
        </div>
      </div>
      <div
        className="pointer-events-none absolute inset-0 ring-1 ring-inset ring-black/10 md:rounded-3xl"
        aria-hidden="true"
      />
      <a
        href="/template.py"
        className="mt-6 inline-flex items-center gap-x-2 rounded-md bg-indigo-600 px-3.5 py-2.5 text-sm font-semibold text-white shadow-sm hover:bg-indigo-500 focus-visible:outline focus-visible:outline-2 focus-visible:outline-offset-2 focus-visible:outline-indigo-600"
      >
        <ArrowDownOnSquareIcon className="-ml-0.5 h-5 w-5" aria-hidden="true" />
        Download Template
      </a>
    </div>
  );
}
