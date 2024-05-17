import { AppRouterPageRoute, withPageAuthRequired } from '@auth0/nextjs-auth0'
import { WithPageAuthRequiredOptions } from '@auth0/nextjs-auth0/client'

const defaultOptions: WithPageAuthRequiredOptions = {
  onRedirecting: () => <h1>Loading...</h1>,
  onError: (error: any) => <h1>Error: {error.message}</h1>,
}

export function authRequired(
  getServerSidePropsFunc: AppRouterPageRoute,
  options: WithPageAuthRequiredOptions = defaultOptions,
) {
  return withPageAuthRequired(getServerSidePropsFunc, options)
}
