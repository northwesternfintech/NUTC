import KeyiWang from "@/app/assets/staffphotos/organizers/wang-ky.jpg";
import ZiyiXia from "@/app/assets/staffphotos/organizers/xia-zy.png";
import KentWu from "@/app/assets/staffphotos/organizers/wu-k.jpg";
import EthanHavemann from "@/app/assets/staffphotos/organizers/wang-ky.jpg";
import SteveEwald from "@/app/assets/staffphotos/organizers/wang-ky.jpg";
import AndrewLi from "@/app/assets/staffphotos/organizers/wang-ky.jpg";
import GavinWang from "@/app/assets/staffphotos/organizers/wang-ky.jpg";
import XiaoyangLiu from "@/app/assets/staffphotos/advisors/liu-xy.png";
import ZihanDing from "@/app/assets/staffphotos/advisors/ding-zh.png";
import Zhaoran from "@/app/assets/staffphotos/advisors/zhaoran.png";
import { StaticImageData } from "next/image";

interface IStaffMember {
    photo: StaticImageData;
    name: string;
    bio: string;
}

interface IStaffCategory {
    category: string;
    members: IStaffMember[]
}

export const ALL_STAFF_ARRAY: IStaffCategory[] = [
    {
        category: "Organizers",
        members: [
            {
                photo: KeyiWang,
                name: "Keyi Wang",
                bio: "Master's candidate at Northwestern University, bachelor at Columbia University. Interested in machine learning and financial engineering. Core member of AI4Finance open-source community, responsible for project maintenance and promotion, including FinRL and FinRL-Meta.",
            },
            {
                photo: ZiyiXia,
                name: "Ziyi Xia",
                bio: "Master's candidate at Columbia University. Interested in financial reinforcement learning, big data. The core maintainer of AI4Finance open-source community, including popular projects: FinRL, FinRL-Meta.",
            },
            {
                photo: KentWu,
                name: "Kent Wu",
                bio: "Master's candidate at Columbia University with a keen interest in financial reinforcement learning and language modeling. He plays a pivotal role as the primary maintainer of the AI4Finance open-source community. This includes renowned projects such as FinRL and FinRL-Meta.",
            },
        ]
    },
    {
        category: "Platform",
        members: [
            {
                photo: EthanHavemann,
                name: "Ethan Havemann",
                bio: "will be written later.",
            },
            {
                photo: SteveEwald,
                name: "Steve Ewald",
                bio: "will be written later",
            },
            {
                photo: AndrewLi,
                name: "Andrew Li",
                bio: "will be written later.",
            },
            {
                photo: GavinWang,
                name: "Gavin Wang",
                bio: "will be written later.",
            },
        ]
    },
    {
        category: "Advisors",
        members: [
            {
                photo: XiaoyangLiu,
                name: "Xiao-Yang Liu",
                bio: "Ph.D candidate, Columbia University. His research interests include deep reinforcement learning, big data, and high-performance computing. He co-authored a textbook on reinforcement learning for cyber-physical systems, a textbook on tensor for data processing. He serves as PC member for NeurIPS, ICML, ICLR, AAAI, IJCAI, AISTATS, KDD, ACM MM, ACM ICAIF, ICASSP, and a Session Chair for IJCAI 2019. He organized the NeurIPS 2020/2021 First/Second Workshop on Quantum Tensor Networks in Machine Learning, IJCAI 2020 Workshop on Tensor Networks Representations in Machine Learning, and NeurIPS 2019/2020 Workshop on Machine Learning for Autonomous Driving.",
            },
            {
                photo: ZihanDing,
                name: "Zihan Ding",
                bio: "Ph.D., Princeton University. His primary research interests are: deep reinforcement learning algorithms and applications. He serves as the reviewer of NeurIPS, ICLR, ICML, CVPR, ICCV, AAAI, AISTATS, RA-L, ICRA, IROS, CISS, AIM. He also served on the organization committee of Human in the Loop Learning (HiLL) Workshop at NeurIPS 2022.",
            },
            {
                photo: Zhaoran,
                name: "Zhaoran Wang",
                bio: "will be written later. PLEASE fix this photo later too.",
            },
        ]
    },
]