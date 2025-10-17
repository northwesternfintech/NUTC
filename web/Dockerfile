# ---- Base ----
# Use a specific Node.js version on a lightweight Alpine Linux base.
FROM node:20-alpine AS base
# Install OpenSSL, which is a required dependency for Prisma's query engine on Alpine.
RUN apk add --no-cache openssl
WORKDIR /app
# Copy package.json and lock file to leverage Docker layer caching.
COPY package*.json ./

# ---- Dependencies ----
# A separate stage for installing only production dependencies.
FROM base AS deps
# Copy the prisma schema BEFORE installing dependencies.
COPY prisma ./prisma
RUN npm install --omit=dev

# ---- Builder ----
# A stage for building the application.
FROM base AS builder
COPY . .
RUN npm install
RUN npx prisma generate
RUN npm run build

# ---- Production Image ----
# The final, lightweight image that will run in production.
FROM base AS runner
# Copy only the necessary files from previous stages.
COPY --from=deps /app/node_modules ./node_modules
COPY --from=builder /app/.next ./.next
COPY --from=builder /app/public ./public
COPY --from=builder /app/prisma ./prisma
COPY package.json .

# Set the environment to production.
ENV NODE_ENV=production

# Expose the port the application listens on.
EXPOSE 3000

# The command to start the application.
CMD ["npm", "start"]

