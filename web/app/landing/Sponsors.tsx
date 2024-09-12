export default function Sponsors() {
  return (
    <div className="bg-gray-900 py-24 sm:py-32">
      <div className="mx-auto max-w-7xl px-6 lg:px-8">
        <div className="-mx-6 grid grid-cols-2 gap-0.5 overflow-hidden sm:mx-0 sm:rounded-2xl md:grid-cols-3">
          <div className="bg-white/5 p-8 sm:p-10">
            <img
              className="max-h-12 w-full object-contain"
              src="https://upload.wikimedia.org/wikipedia/commons/thumb/1/17/IMC_Logo.svg/2560px-IMC_Logo.svg.png"
              alt="IMC"
              width={158}
              height={48}
            />
          </div>
          <div className="bg-white/5 p-6 sm:p-8">
            <img
              className="max-h-16 w-full object-contain"
              src="https://nu-fintech.web.app/static/media/nufticon.8c88d1dc10279d510c78.png"
              alt="NUFT"
              width={158}
              height={48}
            />
          </div>
          <div className="bg-white/5 p-6 sm:p-10">
            <img
              className="max-h-12 w-full object-contain"
              src="https://crypto.marketswiki.com/images/f/f4/DRW_logo.png"
              alt="DRW"
              width={158}
              height={48}
            />
          </div>
        </div>
      </div>
    </div>
  );
}
