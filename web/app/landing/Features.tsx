import {
  PresentationChartLineIcon,
  UserGroupIcon,
  TrophyIcon,
} from '@heroicons/react/20/solid'

import Image from 'next/image'

const features = [
  {
    name: 'Build and Compete.',
    description:
      'Build trading algorithms on provided Python templates and watch PnL in real-time.',
    icon: PresentationChartLineIcon,
  },
  {
    name: 'Meet Sponsors.',
    description:
      'Meet our in-person sponsors for a chance to learn about firms and recruitment.',
    icon: UserGroupIcon,
  },
  {
    name: 'Win Prizes.',
    description:
      'We award a variety of cash and other prizes for top placement in all cases.',
    icon: TrophyIcon,
  },
]

export default function Example() {
  return (
    <div className='overflow-hidden bg-gray-900 py-24 sm:py-32'>
      <div className='mx-auto max-w-7xl px-6 lg:px-8'>
        <div className='mx-auto grid max-w-2xl grid-cols-1 gap-x-8 gap-y-16 sm:gap-y-20 lg:mx-0 lg:max-w-none lg:grid-cols-2'>
          <div className='lg:pr-8 lg:pt-4'>
            <div className='lg:max-w-lg'>
              <p className='mt-2 text-3xl font-bold tracking-tight text-white sm:text-4xl'>
                The Thrill of the Market
              </p>
              <p className='mt-6 text-lg leading-8 text-gray-300'>
                Build your trading algorithm and watch it compete against market
                forces and other competitors in real time
              </p>
              <dl className='mt-10 max-w-xl space-y-8 text-base leading-7 text-gray-300 lg:max-w-none'>
                {features.map((feature) => (
                  <div key={feature.name} className='relative pl-9'>
                    <dt className='inline font-semibold text-white'>
                      <feature.icon
                        className='absolute left-1 top-1 h-5 w-5 text-indigo-500'
                        aria-hidden='true'
                      />
                      {feature.name}
                    </dt>{' '}
                    <dd className='inline'>{feature.description}</dd>
                  </div>
                ))}
              </dl>
            </div>
          </div>
          <div className='relative'>
            <Image
              src='/images/crowd.jpeg'
              alt='Product screenshot'
              className='w-[48rem] max-w-none rounded-xl shadow-xl ring-1 ring-white/10 sm:w-[57rem] md:-ml-4 lg:-ml-0'
              width={2432}
              height={1442}
            />
            <span className='absolute hidden xl:block bottom-[-30px] -right-12 text-white text-sm font-light italic'>
              Spring 2024
            </span>
          </div>
        </div>
      </div>
    </div>
  )
}
