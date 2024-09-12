import Hero from "./landing/Hero";
import Sponsors from "./landing/Sponsors";
import Features from "./landing/Features";
import Updates from "./landing/Updates";
import Footer from "./landing/Footer";
import Stats from "./landing/Stats";
import FAQ from "./landing/FAQ";
const LandingPage = () => {
  return (
    <div>
      <Hero />
      <Sponsors />
      <Features />
      <Stats />
      <FAQ />
      <Updates />
      <Footer />
    </div>
  );
};

export default LandingPage;
