"use client";
import React, { ReactNode } from "react";
import Figure1 from "@/app/assets/figures/fig1.png";
import Table1 from "@/app/assets/figures/table1.png";
import Image from "next/image";
import { ALL_STAFF_ARRAY } from "./staff";
import { NextPage } from "next";

interface IAboutUsSection {
  title: string;
  children: ReactNode;
}

interface IAboutUsH {
  children: ReactNode;
}

interface IAboutUsFigure {
  children: ReactNode;
  caption: string;
  fig_number: string;
}

interface IAboutUsOrganizerHuman {
  children: ReactNode; // ideally a photo lol
  name: string;
  bio: string;
}

const AboutUsOrganizerHuman: React.FC<IAboutUsOrganizerHuman> = ({children, name, bio}) => {
  return (
    <div className="grid grid-cols-1 md:grid-cols-7 gap-4 mt-4">
      <div className="col-span-1 md:col-span-2">
        {children}
      </div>
      <div className="col-span-1 md:col-span-5">
        <p>
          <span className="font-bold">{name}</span> {bio}
        </p>
      </div>
    </div>
  );
}

const AboutUsH3: React.FC<IAboutUsH> = ({children}) => {
  return (
    <h2 className="text-base font-semibold leading-8 mt-4 text-white">
      {children}
    </h2>
  );
}

const AboutUsH4: React.FC<IAboutUsH> = ({children}) => {
  return (
    <h2 className="text-base font-semibold underline leading-8 text-white">
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
      <p className="mt-2 mb-2 text-sm leading-6 text-gray-400">
        {fig_number}: {caption}
      </p>
    </div>
  );
}

export default function AboutUs() {
  return (
    <div>
      <div className="space-y-12 p-3">
        <AboutUsSection title="Home">
          <AboutUsH3>
            Overview
          </AboutUsH3>
          <p className="mt-2 mb-2 text-sm leading-6 text-gray-400">
            Financial reinforcement learning (FinRL), as an interdisciplinary field of finance and reinforcement learning, has been recognized as a promising approach to financial tasks. Over the past decade, deep reinforcement learning (DRL) has achieved remarkable success in solving complex problems across a variety of domains, including robotics, gaming, and large language models like ChatGPT and GPT-4. The success of DRL has also led to its application in finance, where it has demonstrated great potential for enhancing the performance of financial tasks, such as portfolio management, option pricing, and algorithmic trading.
          </p>
          <p className="mt-2 mb-2 text-sm leading-6 text-gray-400">
            The FinRL contest is a competition that explores and evaluates the capability of machine learning methods in finance. However, developing machine learning algorithms for financial data presents unique challenges:
          </p>
          <ol className="mt-2 mb-2 text-sm leading-6 text-gray-400 list-decimal">
            <li><span className="font-bold">Dynamic Transitions and Data Noise</span>: Financial datasets are constantly evolving, making it difficult to capture the underlying dynamics. Moreover, financial data is often susceptible to noise and anomalies.</li>
            <li><span className="font-bold">Partial Observability</span>: No traders can access complete market-influencing information due to the gigantic, complex, and unpredictable nature of financial systems, where unforeseen events like natural disasters, policy changes, and consumer behavior shifts can have hard-to-anticipate effects.</li>
            <li><span className="font-bold">Other Complex Behaviors</span>: Many other factors can cause complicated and unpredictable behaviors, such as the market’s decentralized nature and the large number of financial instruments available.</li>
          </ol>
          <p className="mt-2 mb-2 text-sm leading-6 text-gray-400">
            The FinRL Contest presents two tasks, data-centric stock trading and real time order execution, aiming to foster innovations toward addressing the above challenges. We welcome students, researchers, and engineers who are passionate about finance and machine learning. And we encourage the development of tailored data processing strategies, novel features, and innovative algorithms that can adapt to changing market conditions and generate superior returns for investors.
          </p>

          <AboutUsH3>
            Data
          </AboutUsH3>
          <p className="mt-2 mb-2 text-sm leading-6 text-gray-400">
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
          <p className="mt-2 mb-2 text-sm leading-6 text-gray-400">
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
          <ol className="mt-2 mb-2 text-sm leading-6 text-gray-400">
            <li><span className="font-bold">Submissions Open</span>: October 25th, 2023</li>
            <li><span className="font-bold">Submissions Deadline</span>: November 12th, 2023</li>
            <li><span className="font-bold">Winners Notification</span>: November 25th, 2023</li>
          </ol>
        </AboutUsSection>

        <AboutUsSection title="Task">
          <p className="mt-2 mb-2 text-sm leading-6 text-gray-400">
            Each team can choose to participate in one or both tasks.
          </p>

          <AboutUsH3>
            Starter Kit
          </AboutUsH3>

          <p className="mt-2 mb-2 text-sm leading-6 text-gray-400">
            The Starter Kit will be available at Github.
          </p>

          <AboutUsH3>
            Task I: Data-Centric Stock Trading
          </AboutUsH3>

          <p className="mt-2 mb-2 text-sm leading-6 text-gray-400">
            This task emphasizes data engineering in building FinRL agents. The contestants are encouraged to devise novel data-centric strategies to clean, transform, and aggregate stock data to improve trading performance, with the model being fixed.
          </p>

          <p className="mt-2 mb-2 text-sm leading-6 text-gray-400">
            A dataset containing OHLCV data for stocks is provided. Contestants are free to design data processing strategies and perform feature engineering, such as constructing new indicators based on existing and/or external market data. Then the contestants are required to:
          </p>

          <ol className="mt-2 mb-2 text-sm leading-6 text-gray-400 list-disc">
            <li>Specify the state space, action space, and reward functions in the environment.</li>
            <li>Ensure that your data pipeline is reproducible with unseen, new data.</li>
            <li>Use the same model design without modification for a fair comparison. Specifically, teams are asked to use the PPO algorithm in the FinRL library with tunable hyperparameters.</li>
          </ol>

          <AboutUsH4>
            Evaluation
          </AboutUsH4>
          <p className="mt-2 mb-2 text-sm leading-6 text-gray-400">
            The final ranking is produced based on the geometric mean of the following metrics:
          </p>

          <ol className="mt-2 mb-2 text-sm leading-6 text-gray-400 list-disc">
            <li>Portfolio cumulative return. It measures the excess returns.</li>
            <li>Sharpe ratio. It takes into account both the returns of the portfolio and the level of risk.</li>
            <li>Max drawdown. It is the portfolio’s largest percentage drop from a peak to a trough in a certain time period, which provides a measure of downside risk.</li>
          </ol>

          <AboutUsH3>
            Task II: Real Time Order Execution
          </AboutUsH3>
          <p className="mt-2 mb-2 text-sm leading-6 text-gray-400">
            This task focuses on building lightweight algorithmic trading systems in a fair environment.
          </p>

          <p className="mt-2 mb-2 text-sm leading-6 text-gray-400">
            A template will be provided for contestants, and each team needs to write their functions in this template to perform order execution. We will provide an exchange of limit order book data to interact with contestants. After the contest, we will conduct real-time trading for all teams&apos; submissions at the same time. We would advise you to keep your algorithms lightweight. 
          </p>
          
          <AboutUsH4>
            Evaluation
          </AboutUsH4>
          <p className="mt-2 mb-2 text-sm leading-6 text-gray-400">
            This contest is judged on pure PnL. Strategies that fail to adhere to the template will be disqualified, as well as any strategies that fail to pass linting. 
          </p>
        </AboutUsSection>

        <AboutUsSection title="Rules and Engagement">
          <p className="mt-2 mb-2 text-sm leading-6 text-gray-400">
            The rules of the FinRL Contest aim to ensure fairness, transparency, and ethical conduct while also encouraging contestants to engage in knowledge-sharing and collaboration within the community:
          </p>
          <ol className="mt-2 mb-2 text-sm leading-6 text-gray-400 list-disc">
            <li>Each team should consist of 1 to 4 people and should only register once. Each participant must join one and only one team.</li>
            <li>Submission files should follow the format of the baseline solution and the requirements.</li>
            <li>Each team can make multiple submissions to each task during the competition. The most recent submission will be used for final evaluation.</li>
            <li>Contestants can communicate with the organizers through Discord or email.</li>
            <li>We embrace the open-source community. At the end of the contest, contestants are encouraged to release their work as open-source.</li>
            <li>Contestants are expected to adhere to the highest standards of ethical conduct and integrity throughout the contest.</li>
          </ol>
        </AboutUsSection>

        <AboutUsSection title="Team Registration">
          <p className="mt-2 mb-2 text-sm leading-6 text-gray-400">
            Team registration is done through the website. Please ensure that all team members input the same team leader. Registrations will be processed within 24 hours. 
          </p>
        </AboutUsSection>

        <AboutUsSection title="Submission">
          <p className="mt-2 mb-2 text-sm leading-6 text-gray-400">
            All submissions are to be submitted through the website as well. Participants may choose to complete either or both tracks of the competition with no penalty. Note that the most recent submission that passes linting for each track will be the one used in final evaluations.
          </p>
        </AboutUsSection>

        <AboutUsSection title="Organizers">
          {ALL_STAFF_ARRAY.map((cat, index) => (
            <div key={`staff-category-${cat.category}-${index}`}>
              <AboutUsH3>
                {cat.category}
              </AboutUsH3>
              {cat.members.map((element, i) => (
                <AboutUsOrganizerHuman name={element.name} bio={element.bio} key={`human-${cat.category}-${element.name}-${i}`}>
                  <Image
                    src={element.photo}
                    alt={element.name}
                    className="w-full"
                  />
                </AboutUsOrganizerHuman>
              ))}
            </div>
          ))}
        </AboutUsSection>

        <AboutUsSection title="Contact">
          <p className="mt-2 mb-2 text-sm leading-6 text-gray-400">
            Contestants can communicate any questions on Discord.
          </p>
          <p className="mt-2 mb-2 text-sm leading-6 text-gray-400">
          Contact email: <a href="mailto:finrlcontest@gmail.com" className="underline">finrlcontest@gmail.com</a>
          </p>
        </AboutUsSection>
      </div>
    </div>
  );
}
