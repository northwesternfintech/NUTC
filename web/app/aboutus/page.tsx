"use client";
import Link from "next/link";
import React, { ReactNode } from "react";
import Figure1 from "@/app/assets/figures/fig1.png";
import Table1 from "@/app/assets/figures/table1.png";
import Image from "next/image";

interface IAboutUsSection {
  title: string;
  children: ReactNode;
}

interface IAboutUsH3 {
  children: ReactNode;
}

interface IAboutUsFigure {
  children: ReactNode;
  caption: string;
  fig_number: string;
}

const AboutUsH3: React.FC<IAboutUsH3> = ({children}) => {
  return (
    <h2 className="text-base font-semibold leading-8 text-white">
      {children}
    </h2>
  );
}

const AboutUsSection: React.FC<IAboutUsSection> = ({title, children}) => {
  return (
    <div className="border-y border-white/10 pb-12 pt-12">
      <h2 className="text-3xl font-bold leading-7 text-white">
        {title}
      </h2>
      <div>
        {children}
      </div>
    </div>
  );
}

const AboutUsFigure: React.FC<IAboutUsFigure> = ({children, caption, fig_number}) => {
  return (
    <div className="border-y border-white/10 pb-12 pt-12">
      {children}
      <p className="mt-2 text-sm leading-6 text-gray-400">
        {fig_number}: {caption}
      </p>
    </div>
  );
}

export const AboutUs: React.FC = () => {
  return (
    <div>
      <div className="space-y-12 p-3">
        <AboutUsSection title="Home">
          <AboutUsH3>
            Overview
          </AboutUsH3>
          <p className="mt-2 text-sm leading-6 text-gray-400">
            Financial reinforcement learning (FinRL), as an interdisciplinary field of finance and reinforcement learning, has been recognized as a promising approach to financial tasks. Over the past decade, deep reinforcement learning (DRL) has achieved remarkable success in solving complex problems across a variety of domains, including robotics, gaming, and large language models like ChatGPT and GPT-4. The success of DRL has also led to its application in finance, where it has demonstrated great potential for enhancing the performance of financial tasks, such as portfolio management, option pricing, and algorithmic trading.
          </p>
          <p className="mt-2 text-sm leading-6 text-gray-400">
            The FinRL contest is a competition that explores and evaluates the capability of machine learning methods in finance. However, developing machine learning algorithms for financial data presents unique challenges:
          </p>
          <ol className="mt-2 text-sm leading-6 text-gray-400 list-decimal">
            <li><span className="font-bold">Dynamic Transitions and Data Noise</span>: Financial datasets are constantly evolving, making it difficult to capture the underlying dynamics. Moreover, financial data is often susceptible to noise and anomalies.</li>
            <li><span className="font-bold">Partial Observability</span>: No traders can access complete market-influencing information due to the gigantic, complex, and unpredictable nature of financial systems, where unforeseen events like natural disasters, policy changes, and consumer behavior shifts can have hard-to-anticipate effects.</li>
            <li><span className="font-bold">Other Complex Behaviors</span>: Many other factors can cause complicated and unpredictable behaviors, such as the market’s decentralized nature and the large number of financial instruments available.</li>
          </ol>
          <p className="mt-2 text-sm leading-6 text-gray-400">
            The FinRL Contest presents two tasks, data-centric stock trading and real time order execution, aiming to foster innovations toward addressing the above challenges. We welcome students, researchers, and engineers who are passionate about finance and machine learning. And we encourage the development of tailored data processing strategies, novel features, and innovative algorithms that can adapt to changing market conditions and generate superior returns for investors.
          </p>

          <AboutUsH3>
            Data
          </AboutUsH3>
          <p className="mt-2 text-sm leading-6 text-gray-400">
            We have more than 30 market data sources to deal with different financial tasks. We hold the data APIs and sample market environments in an open-source repository, FinRL-Meta, as shown in Figure 1. Contestants are welcome to explore and use in the FinRL Contest.
          </p>
          <AboutUsFigure caption="Market data sources of FinRL-Meta" fig_number="Figure 1">
            <Image
              src={Figure1}
              alt="fig1"
            />
          </AboutUsFigure>

          <AboutUsH3>
            Environment
          </AboutUsH3>
          <p className="mt-2 text-sm leading-6 text-gray-400">
            With a deep reinforcement learning approach, market datasets are processed into gym-style market environments. Table 1 lists the state spaces, action spaces, and reward functions of different FinRL applications. A state shows how an agent perceives a market situation. Facing a state, the agent can take an action from the action set, which may vary according to the financial tasks. Reward is an incentive mechanism for an agent to learn a better policy. Contestants will specify the state space, action space, and reward functions in the environment for Task I Data-Centric Stock Trading.
          </p>
          <AboutUsFigure caption="List of state space, action space, and reward function" fig_number="Table 1">
            <Image
              src={Table1}
              alt="table1"
            />
          </AboutUsFigure>

          <AboutUsH3>
            Timeline
          </AboutUsH3>
          <ol className="mt-2 text-sm leading-6 text-gray-400">
            <li><span className="font-bold">Submissions Open</span>: October 25rd, 2023</li>
            <li><span className="font-bold">Submissions Deadline</span>: November 12th, 2023</li>
            <li><span className="font-bold">Winners Notification</span>: November 25th, 2023</li>
          </ol>
        </AboutUsSection>

        <AboutUsSection title="Task">
          
        </AboutUsSection>
      </div>
    </div>
  );
}

export default AboutUs;
